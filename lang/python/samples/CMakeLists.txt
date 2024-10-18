set(python_samples_measurement
  measurement/low_level_rw.py
  measurement/low_level_rw_protobuf.py
  measurement/measurement_read.py
)

set(python_samples_core
  benchmark/latency_rec.py
  benchmark/latency_rec_cb.py
  benchmark/latency_snd.py
  monitoring/monitoring.py
  monitoring/monitoring_json.py
  pubsub/binary_rec_cb.py
  pubsub/binary_snd.py
  pubsub/protobuf_rec.py
  pubsub/protobuf_rec_cb.py
  pubsub/protobuf_snd.py
  pubsub/string_rec.py
  pubsub/string_rec_cb.py
  pubsub/string_snd.py
  pubsub/_protobuf/animal.proto
  pubsub/_protobuf/animal_pb2.py
  pubsub/_protobuf/binary_rec.py
  pubsub/_protobuf/house.proto
  pubsub/_protobuf/house_pb2.py
  pubsub/_protobuf/person.proto
  pubsub/_protobuf/person_pb2.py
  service/minimal_service_client.py
  service/minimal_service_server.py
)

function(add_sample_to_solution python_filenname)
  cmake_path(GET python_filenname PARENT_PATH subfolder)
  cmake_path(GET python_filenname STEM        stem)

  ecal_python_add_sample(TARGET_NAME py_${stem} PY_FILE ${python_filenname})
  set_property(TARGET py_${stem} PROPERTY FOLDER samples/python/${subfolder})
endfunction()

if(HAS_HDF5)
  foreach(python_sample ${python_samples_measurement})
    add_sample_to_solution(${python_sample})
  endforeach()
endif()

foreach(python_sample ${python_samples_core})
  add_sample_to_solution(${python_sample})
endforeach()