import os
import sys
import random
import time
import ecal.measurement.hdf5 as ecalhdf5
import ecal.proto.helper as pb_helper
sys.path.insert(1, os.path.join(sys.path[0], '../../pubsub/protobuf/_protobuf'))
import person_pb2

def main():
    # We create an ecalhdf5 file and write a protobuf instance in it
    ENTRY_COUNT = 40

    # Declare an instance of protobuf
    person = person_pb2.Person()
    # File and Channel information
    output_dir = "ecalhdf5_protobuf_rw_meas_folder"
    file_name = "measurement"
    channel_name = "person"
    channel = ecalhdf5.Channel(channel_name, pb_helper.get_descriptor_from_type(person_pb2.Person),
                                 "proto:" + person_pb2.Person.DESCRIPTOR.full_name)

    max_size_per_file = 500
    meas = ecalhdf5.Meas(output_dir, 1)
    meas.set_file_base_name(file_name)
    meas.set_max_size_per_file(max_size_per_file)

    meas.set_channel_description(channel.name, channel.description)
    meas.set_channel_type(channel.name, channel.type)

    print("Creating {}/{}.hdf5 \n".format(output_dir, file_name))

    names = ["John", "Alice"]
    genders = [person_pb2.Person.MALE, person_pb2.Person.FEMALE]

    # Timestamp is in microseconds
    initial_timestamp = int(time.time()) * 10**6
    for i in range(ENTRY_COUNT):
        timestamp = initial_timestamp + i * 10**6
        index = (random.randint(1, ENTRY_COUNT * 42)) % 2
        # Dummy Person Data
        person.id = i
        person.name = names[index]
        person.stype = genders[index]
        person.email = person.name + "@example.com"
        person.dog.name = "Brandy"
        person.house.rooms = 4
        meas.add_entry_to_file(person.SerializeToString(), timestamp, timestamp + 1337 * 42, channel_name)

        print("  person_id: {}\tchannel: {}\tperson_name: {}\tperson_gender: {}".format(person.id, channel_name, person.name, person.SType.Name(person.stype)))

    if meas.is_ok() == False:
        print("Write error!")
        sys.exit()

    meas.close()

    print("")
    print("******************************************************************************\n")
    # Now we read from the previously generated hdf5 file
    meas = ecalhdf5.Meas(output_dir, 0)

    if meas.is_ok() == False:
        print("Read error!")
        sys.exit()

    # Some information about our channel
    print("Reading {}\n".format(output_dir))
    print("  File version:        {}".format(meas.get_file_version()))
    print("  Channels No:         {}\n".format(len(meas.get_channel_names())))
    print("  Channel:            \n")

    channel_names_set = meas.get_channel_names()
    for channel_name in channel_names_set:
        print("    Name:          {}".format(channel_name))
        print("    Type:          {}".format(meas.get_channel_type(channel_name)))
        print("    Description:   {}".format(meas.get_channel_description(channel_name)))
        print("    Min timestamp: {}".format(meas.get_min_timestamp(channel_name)))
        print("    Max timestamp: {}".format(meas.get_max_timestamp(channel_name)))

    entries_info_read = meas.get_entries_info(channel_name)
    print("    Entries count: {}\n".format(len(entries_info_read)))

    for entry_read in entries_info_read:
        data_size = meas.get_entry_data_size(entry_read['id'])
        print("    snd_timestamp: {}\trcv_timestamp: {}\tsize[bytes]: {}".format(entry_read['snd_timestamp'],
                                                                                 entry_read['rcv_timestamp'],
                                                                                 data_size))
        entry_data = meas.get_entry_data(entry_read['id'])
        # Create a new instance of the protobuf message
        protobuf_message = person_pb2.Person()
        # Parse the ecalhdf5 data into the protobuf message
        protobuf_message.ParseFromString(entry_data)
        print("    entry_id: {}\tperson_id: {}\tname: {}\tgender: {}".format(entry_read["id"],
                                                                         protobuf_message.id,
                                                                         protobuf_message.name,
                                                                         person.SType.Name(protobuf_message.stype)))
        print()


if __name__ == "__main__":
    main()
