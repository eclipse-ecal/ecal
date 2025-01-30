set(python_samples_measurement
  measurement/low_level_rw.py
  measurement/low_level_rw_protobuf.py
  measurement/measurement_read.py
)

set(python_samples_core
  core/benchmark/latency_rec.py
  core/benchmark/latency_rec_cb.py
  core/benchmark/latency_snd.py
  core/monitoring/monitoring.py
  core/monitoring/monitoring_json.py
  core/pubsub/binary_rec.py
  core/pubsub/binary_rec_cb.py
  core/pubsub/binary_snd.py
  core/pubsub/protobuf_rec.py
  core/pubsub/protobuf_rec_cb.py
  core/pubsub/protobuf_snd.py
  core/pubsub/string_rec.py
  core/pubsub/string_rec_cb.py
  core/pubsub/string_snd.py
  core/service/minimal_service_client.py
  core/service/minimal_service_server.py
)

function(add_sample_to_solution python_filenname)
  cmake_path(GET python_filenname PARENT_PATH subfolder)
  cmake_path(GET python_filenname STEM        stem)
  cmake_path(ABSOLUTE_PATH python_filenname)

  ecal_python_add_sample(TARGET_NAME py_${stem} PY_FILE ${python_filenname})
  set_property(TARGET py_${stem} PROPERTY FOLDER samples/python/${subfolder})
endfunction()


if (CMAKE_GENERATOR MATCHES "Visual Studio")
if(ECAL_PYTHON_USE_HDF5)
  foreach(python_sample ${python_samples_measurement})
    add_sample_to_solution(${python_sample})
  endforeach()
endif()

foreach(python_sample ${python_samples_core})
  add_sample_to_solution(${python_sample})
endforeach()
endif()