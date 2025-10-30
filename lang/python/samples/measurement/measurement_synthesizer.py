# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2025 Aumovio
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

import os
import random
import yaml
import sys

from typing import NamedTuple, List

import ecal.measurement.hdf5 as ecalhdf5

class RandomMemory(object):
    def __init__(self, size: int):
        self._size = size
        self._memory = self._generate()

    def _generate(self) -> bytes:
        return os.urandom(self._size)

    def regenerate(self):
        """Regenerates the byte array with new random content."""
        self.memory = self._generate()

    
    def get_bytes(self, count: int) -> bytes:
        """Returns a random contiguous subset of the byte array."""
        if count > self._size:
            raise ValueError("Requested count exceeds the length of the byte array.")
        start_index = random.randint(0, self._size - count)
        return (memoryview(self._memory)[start_index:start_index + count]).tobytes()
        

class Frame(NamedTuple):
    timestamp: int
    send_timestamp: int
    size: int
    counter: int
    name: str
    
class FrameParameter(NamedTuple):
    size: int
    size_deviation: int
    cycle_time: int
    cycle_time_deviation: int
    drop_probability: float
    send_receive_offset: int
    send_receive_offset_deviation: int
    
class MeasurementParameter(NamedTuple):
    output_directory: str
    file_base_name: str
    max_size_per_file: int
    

class Channel(object):
    def __init__(self, name: str, timestamp_begin: int, timestamp_end: int, frame_parameter: FrameParameter):
        self._name = name
        self._frame_parameter = frame_parameter
        self._timestamp_begin = timestamp_begin
        self._timestamp_end = timestamp_end
    
    def get_name(self):
        """Returns the name of the channel"""
        return self._name
    
    def generate(self) -> List[Frame]:
        """Returns a generated frames for the channel based on the configured parameters."""
        frame_count = (self._timestamp_end - self._timestamp_begin) // self._frame_parameter.cycle_time;
        timestamp = self._timestamp_begin
        counter = 0
        frames = []
        for i in range(frame_count):
            timestamp = round(random.gauss(timestamp, self._frame_parameter.cycle_time_deviation))
            size = round(random.gauss(self._frame_parameter.size, self._frame_parameter.size_deviation))
            send_timestamp = timestamp - round(random.gauss(self._frame_parameter.send_receive_offset, self._frame_parameter.send_receive_offset_deviation))
            if random.random() >= self._frame_parameter.drop_probability:
                frames.append(Frame(name=self._name, timestamp=timestamp, send_timestamp=send_timestamp, counter=counter, size=size))
            counter += 1
            timestamp += self._frame_parameter.cycle_time
        return frames


def load_settings_from_yaml(yaml_path: str) -> List[Channel]:
    with open(yaml_path, 'r') as file:
        data = yaml.safe_load(file)

    channels = [
        Channel(
            name=channel['name'],
            timestamp_begin=channel['timestamp_begin'],
            timestamp_end=channel['timestamp_end'],
            frame_parameter=FrameParameter(**channel['frame_parameter'])
        )
        for channel in data['channels']
    ]
    
    measurement_parameter = MeasurementParameter(
            output_directory=data["output_directory"],
            file_base_name=data["file_base_name"],
            max_size_per_file=data["max_size_per_file"]
        )
    return (channels, measurement_parameter)

class ProgressSpin(object):
    _spin_characters = ['-', '\\', '|', '/']

    def __init__(self, chunk_count, step_count):
        self._counter = 0
        self._subcounter = 0
        self._steps_per_chunk = step_count // chunk_count
        
    def reset(self):
        """Resets the status of the progress spin"""
        self._counter = 0
        self._subcounter = 0
        print("", flush=True)
    
    def next(self):
        """Calcuclates and displays the netx progress iteration"""
        if self._counter % 500 == 0:
            print(f"{ProgressSpin._spin_characters[self._subcounter % len(ProgressSpin._spin_characters)]}\b", end='', flush=True)
            self._subcounter += 1            
        self._counter += 1
        if self._counter % self._steps_per_chunk == 0: print('#', end='', flush=True)

def main(argv):
    if len(argv) != 2:
        
        print(f"Usage: {argv[0]} <yaml settings path>", file=sys.stderr)
        print("", file=sys.stderr)
        print(f"Synthesizer for generating artifical eCAL HDF5 measurements.", file=sys.stderr)
        print(f"MIT License, Copyright (c) 2025 Kristof Hannemann", file=sys.stderr)
        
        return 1
        
    settings = load_settings_from_yaml(argv[1])
    print(f"Settings loaded from {argv[1]}.")
    
    measurement = ecalhdf5.Meas(settings[1].output_directory, 1)
    measurement.set_file_base_name(settings[1].file_base_name)
    measurement.set_max_size_per_file(settings[1].max_size_per_file)
    frames = []
    
    for channel in settings[0]:
        measurement.set_channel_description(channel._name, b"")
        measurement.set_channel_type(channel._name, "")
        channel_frames = channel.generate()
        print(f"Prepared {len(channel_frames)} frames for channel '{channel.get_name()}'.")
        frames.extend(channel_frames)
    frames = sorted(frames, key=lambda frame: frame.timestamp)
    memory = RandomMemory(max(frame.size for frame in frames) * 10)

    print(f"Estimated measurement size ~{sum(frame.size for frame in frames) // 1000 // 1000} MByte.")
    
    print("Writing measurement...")
    
    progress_spin = ProgressSpin(20, len(frames))
    
    for frame in frames:
        measurement.add_entry_to_file(memory.get_bytes(frame.size), frame.send_timestamp, frame.timestamp, frame.name, frame.counter)
        progress_spin.next()
    
    measurement.close()
    
    return 0

if __name__ == '__main__':

    sys.exit(main(sys.argv))
