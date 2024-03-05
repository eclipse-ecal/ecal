# ecal-perftool

The ecal-perftool is a simple application to estimate the performance of eCAL pub-sub connections using dummy-data being published a at a constant frequency.

## Usage

```
Usage:
  ecal_sample_perftool pub <topic_name> <frequency_hz> <payload_size_bytes> [options]
or:
  ecal_sample_perftool sub <topic_name> [callback_delay_ms] [options]

Options:
  -q, --quiet:     Do not print any output
  -v, --verbose:   Print all measured times for all messages
      --busy-wait: Busy wait when receiving messages (i.e. burn CPU). For subscribers only.
      --hickup <after_ms> <delay_ms>: Further delay a single callback. For subscribers only.
```

## Output

**Publisher**:

```
[ 78436.510] | cnt:    9 | loop_dt(ms) mean:  99.954 [  99.587, 100.001] | loop_freq(Hz):   10.0 | snd_dt(ms) mean:   0.001 [   0.001,   0.001]
[ 78437.525] | cnt:   10 | loop_dt(ms) mean: 100.000 [  99.999, 100.001] | loop_freq(Hz):   10.0 | snd_dt(ms) mean:   0.001 [   0.000,   0.001]
[ 78438.538] | cnt:   10 | loop_dt(ms) mean: 100.001 [  99.999, 100.013] | loop_freq(Hz):   10.0 | snd_dt(ms) mean:   0.001 [   0.000,   0.001]
[ 78439.545] | cnt:   10 | loop_dt(ms) mean:  99.999 [  99.987, 100.002] | loop_freq(Hz):   10.0 | snd_dt(ms) mean:   0.001 [   0.001,   0.001]
[ 78440.551] | cnt:   10 | loop_dt(ms) mean: 100.000 [  99.999, 100.001] | loop_freq(Hz):   10.0 | snd_dt(ms) mean:   0.001 [   0.001,   0.001]
```

- `[ xxx]`: Log system time
- `cnt`: Amount of messages sent since last log output
- `loop_dt`: Duration of publishing loop, consisting of mean `mean [min, max]` in milliseconds
- `loop_freq`:  computed loop frequency in Hz
- `snd_dt`: Duration of the eCAL `CPublisher::Send()` call only, consisting of `mean [min, max]` in milliseconds

**Subscriber**

```
[ 78927.089] | cnt:   10 | lost:    0 | msg_dt(ms) mean:  99.997 [  99.967, 100.019] | msg_freq(Hz):   10.0
[ 78928.103] | cnt:   10 | lost:    0 | msg_dt(ms) mean: 100.000 [  99.964, 100.031] | msg_freq(Hz):   10.0
[ 78929.104] | cnt:   10 | lost:    0 | msg_dt(ms) mean:  99.998 [  99.966, 100.039] | msg_freq(Hz):   10.0
[ 78930.117] | cnt:   10 | lost:    0 | msg_dt(ms) mean: 100.001 [  99.993, 100.010] | msg_freq(Hz):   10.0
[ 78931.132] | cnt:   10 | lost:    0 | msg_dt(ms) mean: 100.000 [  99.966, 100.030] | msg_freq(Hz):   10.0
```

- `[ xxx]`: Log system time
- `cnt`: Amount of received sent since last log output
- `lost`: Amount of dropped messages since the last log output. Determined by comparing the native eCAL message counter of each message to the previous.
- `msg_dt`: Duration between the received messages, consisting of  `mean [min, max]` in milliseconds
- `msg_freq`: Computed message frequency in Hz
