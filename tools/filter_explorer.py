"""A tool for exploring the frqeuency response of simple digital filters."""
from matplotlib import pyplot
import numpy
from typing import Tuple, List, Text


class Filter(object):
    def ProcessSample(self, sample: float) -> float:
        """Processes a single sample with this filter.

        Args:
            sample: A single time-domain sample of the input signal.

        Returns:
            The output sample with the filter applied."""
        return 0.0


class FirFilter(Filter):
    def __init__(self, taps: List[float]):
        """Initializes an FirFilter with the given time-domain taps.

        Args:
            taps: Time-domain taps for the filter."""
        self.taps = taps
        self.Reset()

    def Reset(self):
        self.buffer = [0] * len(self.taps)

    def ProcessSample(self, sample: float) -> float:
        """Processes a single sample with this filter.

        Args:
            sample: A single time-domain sample of the input signal.

        Returns:
            The output sample with the filter applied."""
        self.buffer = [sample] + self.buffer[:-1]
        return numpy.dot(self.buffer, self.taps)


class IirFilter(Filter):
    def __init__(self, x_taps: List[float], y_taps: List[float]):
        """Initializes an FirFilter with the given time-domain taps.

        Args:
            taps: Time-domain taps for the filter."""
        self.x_taps = x_taps
        self.y_taps = y_taps
        self.Reset()

    def Reset(self):
        self.x_buffer = [0] * len(self.x_taps)
        self.y_buffer = [0] * len(self.y_taps)

    def ProcessSample(self, sample: float) -> float:
        """Processes a single sample with this filter.

        Args:
            sample: A single time-domain sample of the input signal.

        Returns:
            The output sample with the filter applied."""
        self.x_buffer = [sample] + self.x_buffer[:-1]
        y = numpy.dot(self.x_buffer, self.x_taps) + numpy.dot(
            self.y_buffer, self.y_taps)
        self.y_buffer = [y] + self.y_buffer[:-1]
        return y


class BodePlotter(object):
    def __init__(self, sampling_rate: float,
                 frequency_range: Tuple[float, float], frequency_step: float):
        """Initializes a BodePlotter instance with the given parameters.

        Args:
            sampling_rate: Sampling rate, in Hz.
            frequency_range: Start and stop frequency, in Hz, for the Bode plot.
            frequency_step: Step, in Hz, between samples on the Bode plot.
        """
        self.sampling_rate = sampling_rate
        self.frequency_range = frequency_range
        self.frequency_step = frequency_step

        self.frequencies = numpy.arange(*frequency_range, frequency_step)

    def Plot(self, test_filter: Filter, num_periods: int = 10) -> None:
        amplitude_response = []

        for frequency in self.frequencies:
            duration = (1 / frequency) * num_periods
            input_signal_timebase = numpy.arange(0, duration,
                                                 1.0 / self.sampling_rate)
            input_signal = numpy.sin(input_signal_timebase * frequency * 2 *
                                     numpy.pi)

            test_filter.Reset()
            output_signal = [
                test_filter.ProcessSample(sample) for sample in input_signal
            ]

            amplitude_response.append(
                numpy.linalg.norm(output_signal) /
                numpy.sqrt(len(output_signal)))

        return self.frequencies, amplitude_response


FILTER_TYPE_BANDPASS = 0
FILTER_TYPE_BANDSTOP = 1


def GenerateIirBandTaps(center_frequency: float, bandwidth: float,
                        filter_type: int) -> Tuple[List[float], List[float]]:
    """Computes taps for a band(pass|stop) IIR filter.

    Args:
        center_frequency: Center frequency of the band filter, normalized by the
                          sampling rate.
        bandwidth: Bandwidth of the band filter, normalized by the sampling
                   rate.
        filter_type: Either FILTER_TYPE_BANDPASS or FILTER_TYPE_BANDSTOP.

    Returns:
        A tuple of feedforward and feedback filter taps.
    """
    R = 1 - 3 * bandwidth
    K = (1 - 2 * R * numpy.cos(2 * numpy.pi * center_frequency) +
         R**2) / (2 - 2 * numpy.cos(2 * numpy.pi * center_frequency))

    if filter_type == FILTER_TYPE_BANDPASS:
        feedforward_taps = [
            1 - K, 2 * (K - R) * numpy.cos(2 * numpy.pi * center_frequency),
            R**2 - K
        ]
        feedback_taps = [
            2 * R * numpy.cos(2 * numpy.pi * center_frequency), -R**2
        ]
    elif filter_type == FILTER_TYPE_BANDSTOP:
        feedforward_taps = [
            K, -2 * K * numpy.cos(2 * numpy.pi * center_frequency), K
        ]
        feedback_taps = [
            2 * R * numpy.cos(2 * numpy.pi * center_frequency), -R**2
        ]
    else:
        raise ValueError("Unknown filter type: {}".format(filter_type))

    return feedforward_taps, feedback_taps


def GenerateIirFilterDeclaration(feedforward_taps: List[float],
                                 feedback_taps: List[float]) -> Text:
    return "IirFilter filter_{{{{{}}}, {{{}}}}};".format(
        ','.join(['%.6ff' % f for f in feedforward_taps]),
        ','.join(['%.6ff' % f for f in feedback_taps]))


if __name__ == '__main__':
    bode_plotter = BodePlotter(44100, (10, 20000), 200)
    filter_frequency_hz = 1600
    bandwidth_hz = 900

    # FIR filter
    filter_timebase = numpy.arange(
        0, numpy.pi * 2,
        numpy.pi * 2 / (bode_plotter.sampling_rate / filter_frequency_hz))
    filter = FirFilter(list(numpy.sin(filter_timebase)))

    pyplot.plot(*bode_plotter.Plot(filter))
    pyplot.show()

    # IIR Filter
    feedforward_taps, feedback_taps = GenerateIirBandTaps(
        filter_frequency_hz / bode_plotter.sampling_rate,
        bandwidth_hz / bode_plotter.sampling_rate, FILTER_TYPE_BANDSTOP)

    print(GenerateIirFilterDeclaration(feedforward_taps, feedback_taps))

    filter = IirFilter(feedforward_taps, feedback_taps)

    pyplot.plot(*bode_plotter.Plot(filter))

    feedforward_taps, feedback_taps = GenerateIirBandTaps(
        filter_frequency_hz / bode_plotter.sampling_rate,
        bandwidth_hz / bode_plotter.sampling_rate, FILTER_TYPE_BANDPASS)

    print(GenerateIirFilterDeclaration(feedforward_taps, feedback_taps))

    filter = IirFilter(feedforward_taps, feedback_taps)

    pyplot.plot(*bode_plotter.Plot(filter))
    pyplot.show()
