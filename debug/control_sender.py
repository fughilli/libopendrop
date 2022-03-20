import mido
import re
import signal
import socket
import sys
import threading
import time
from typing import Dict, List, Tuple

from absl import flags

from debug.control_pb2 import Control, Button

FLAGS = flags.FLAGS

flags.DEFINE_boolean("list", False,
                     "List available MIDI inputs, and then exit")
flags.DEFINE_string("input_filter", None, "Regex to filter MIDI inputs by")
flags.DEFINE_boolean("verbose", False, "")


def make_key(msg):
    control = 0
    note = 0
    if 'control' in msg.dict():
        control = msg.control
    if 'note' in msg.dict():
        note = msg.note
    return f'{msg.type:s}:{msg.channel:d}:{control:d}:{note:d}'


def any_value(msg):
    if 'value' in msg.dict():
        return msg.value
    if 'velocity' in msg.dict():
        return msg.velocity
    if 'pitch' in msg.dict():
        return msg.pitch
    return 0


class Runner:
    def __init__(self):
        self.control_ranges: Dict[str, Tuple[int, int]] = {}
        self.control_values: Dict[str, float] = {}
        self.sorted_control_keys: List[str] = []
        self.buttons: List[Button] = []
        self.inport = None

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server_tuple = ('127.0.0.1', 9944)

    def start(self):
        self.should_exit = False
        self.thread = threading.Thread(target=self.run)
        self.thread.start()

    def update_value(self, key, value):
        ranges = None
        if key not in self.control_ranges:
            self.control_ranges[key] = (value, value)
        else:
            ranges = self.control_ranges[key]
            if value < ranges[0]:
                ranges = (value, ranges[1])
            if value > ranges[1]:
                ranges = (ranges[0], value)
            self.control_ranges[key] = ranges

        if (ranges is None) or (ranges[0] == ranges[1]):
            self.control_values[key] = 0
            return

        self.control_values[key] = (value - ranges[0]) / (ranges[1] -
                                                          ranges[0])

    def maybe_hash_value(self, msg):
        key = make_key(msg)
        if key not in self.sorted_control_keys:
            self.sorted_control_keys.append(key)
            self.sorted_control_keys = sorted(self.sorted_control_keys)
        self.update_value(key, any_value(msg))

    def stop(self):
        self.should_exit = True
        if self.inport is not None:
            print("Closing port")
            self.inport.close()
        self.thread.join()

    def send_control(self):
        control = Control()
        for k, v in self.control_values.items():
            control.control[k] = v
        control.buttons.extend(self.buttons)
        self.buttons = []
        self.socket.sendto(control.SerializeToString(), self.server_tuple)

    def initialize_device(self):
        input_names = mido.get_input_names()

        input_name = list(
            filter((lambda name: re.match(FLAGS.input_filter, name)),
                   input_names))[0]

        print(f"Opening input: \"{input_name:s}\"")

        return mido.open_input(input_name)

    def inport_callback(self, msg):
        if msg.type == 'reset':
            return
        if msg.type.startswith('note'):
            button = Button()
            button.channel = msg.channel * 1000 + msg.note
            button.state = (Button.State.PRESSED if msg.type == 'note_on' else
                            Button.State.RELEASED)
            button.velocity = msg.velocity
            self.buttons.append(button)
        else:
            self.maybe_hash_value(msg)
        self.send_control()

    def run(self):
        self.inport = self.initialize_device()
        self.inport.callback = self.inport_callback

        while not self.should_exit:
            time.sleep(1)

        self.inport.callback = None
        print("Exiting")


runner = Runner()


def start():
    if FLAGS.list:
        input_listing = '\n'.join(mido.get_input_names())
        print(f"Available inputs:\n{input_listing:s}")
        return

    print("Runner start")
    runner.start()


def stop():
    print("Runner stop")
    runner.stop()


def handle_sigint(x, y):
    stop()


if __name__ == '__main__':
    FLAGS(sys.argv)

    signal.signal(signal.SIGINT, handle_sigint)

    start()
