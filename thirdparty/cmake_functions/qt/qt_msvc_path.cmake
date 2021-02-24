# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

macro(autodetect_qt5_msvc_dir)
	SET(QT_MISSING True)
	# msvc only; mingw will need different logic
	IF(MSVC)
		message(STATUS "Trying to auto-detect best Qt5 Version")
		# look for user-registry pointing to qtcreator
		GET_FILENAME_COMPONENT(QTCREATOR_BIN [HKEY_CURRENT_USER\\Software\\Classes\\Applications\\QtProject.QtCreator.pro\\shell\\Open\\Command] DIRECTORY)
		# get root path so we can search for 5.3, 5.4, 5.5, etc
		STRING(REPLACE "/Tools" ";" QT_BIN "${QTCREATOR_BIN}")

		LIST(GET QT_BIN 0 QT_INSTALL_DIRECTORY)
				
		# Collect all Qt5 directories
		FILE(GLOB QT_VERSION_DIRECTORIES "${QT_INSTALL_DIRECTORY}/5.*")
		
		# Compute the MSVC Version as year (e.g. 2015, 2017, 2019...) and aim for upwards compatibility
		if (MSVC_VERSION LESS 1900)
			message(FATAL_ERROR "ERROR: MSVC_VERSION  is $MSVC_VERSION, which indicates Visual Studio < 2015. Only Visual Studio 2015 and up are supported.")			
		endif()
		
		if (MSVC_VERSION LESS_EQUAL "1909")
			set(MSVC_YEAR "2015")
		elseif (MSVC_VERSION LESS_EQUAL "1919")
			set(MSVC_YEAR "2017")
		elseif (MSVC_VERSION LESS_EQUAL "1929")
			set(MSVC_YEAR "2019")
		else () # Assume each 2 years a new Visual Studio Version with a range of 10 MSVC_VERSIONs
			MATH(EXPR MSVC_YEAR "2015 + ((${MSVC_VERSION} - 1900) / 10) * 2")
		endif ()
		
		message(STATUS "Detected Visual Studio ${MSVC_YEAR} (from MSVC_VERSION ${MSVC_VERSION})")
				
		# Iterate over all Qt Versions and pick the best
		set(BEST_QT_DIRECTORY "")
		set(BEST_QT_MSVC_YEAR 0)
		set(BEST_QT_MAJOR 0)
		set(BEST_QT_MINOR 0)
		set(BEST_QT_PATCH 0)
		
		foreach (QT_DIRECTORY ${QT_VERSION_DIRECTORIES})
			# Get last component of path, which is the qt version
			get_filename_component(QT_VERSION_STRING "${QT_DIRECTORY}" NAME)
			
			# Split the string, to get the major, minor and patch
			STRING(REPLACE "." ";" QT_VERSION_COMPONENT_LIST "${QT_VERSION_STRING}")

			LIST(GET QT_VERSION_COMPONENT_LIST 0 QT_MAJOR)
			LIST(GET QT_VERSION_COMPONENT_LIST 1 QT_MINOR)
			LIST(GET QT_VERSION_COMPONENT_LIST 2 QT_PATCH)

			# Check if there is an installation we can use
			if (CMAKE_CL_64)
				FILE(GLOB QT_MSVC_DIRECTORIES "${QT_DIRECTORY}/msvc20[0-9][0-9]_64")
			else ()
				FILE(GLOB QT_MSVC_DIRECTORIES "${QT_DIRECTORY}/msvc20[0-9][0-9]")
			endif()

			# Iterate over all msvc directories and check for the best matching one
			foreach (QT_MSVC_DIR ${QT_MSVC_DIRECTORIES})
				STRING(REPLACE "//" "/"  QT_MSVC_DIR "${QT_MSVC_DIR}")
				get_filename_component(QT_MSVC_DIR_NAME "${QT_MSVC_DIR}" NAME)
				
				STRING(REPLACE "msvc" "" QT_MSVC_YEAR "${QT_MSVC_DIR_NAME}")
				if (CMAKE_CL_64)
					STRING(REPLACE "_64" "" QT_MSVC_YEAR "${QT_MSVC_YEAR}")
				endif()
				
				if (("${QT_MSVC_YEAR}" LESS_EQUAL "${MSVC_YEAR}") AND (${QT_MSVC_YEAR} GREATER_EQUAL "2015"))
					# At this point we have found a version that should be usable. Let's check if it is better than the last one we found!
					message(STATUS "Found Qt5 candidate at ${QT_MSVC_DIR}")
					if ( ("${QT_MAJOR}" GREATER "${BEST_QT_MAJOR}") OR
						(("${QT_MAJOR}" EQUAL   "${BEST_QT_MAJOR}") AND ("${QT_MINOR}" GREATER "${BEST_QT_MINOR}")) OR
						(("${QT_MAJOR}" EQUAL   "${BEST_QT_MAJOR}") AND ("${QT_MINOR}" EQUAL   "${BEST_QT_MINOR}") AND ("${QT_PATCH}" GREATER "${BEST_QT_PATCH}")) OR
						(("${QT_MAJOR}" EQUAL   "${BEST_QT_MAJOR}") AND ("${QT_MINOR}" EQUAL   "${BEST_QT_MINOR}") AND ("${QT_PATCH}" EQUAL   "${BEST_QT_PATCH}") AND ("${QT_MSVC_YEAR}" GREATER "${BEST_QT_MSVC_YEAR}"))
					   )
					   
						set(BEST_QT_DIRECTORY "${QT_MSVC_DIR}")
						set(BEST_QT_MSVC_YEAR "${QT_MSVC_YEAR}")
						set(BEST_QT_MAJOR "${QT_MAJOR}")
						set(BEST_QT_MINOR "${QT_MINOR}")
						set(BEST_QT_PATCH "${QT_PATCH}")
					   
					endif()
					
				endif()
				
			endforeach()
		endforeach()

		if ("${BEST_QT_DIRECTORY}" STREQUAL "")
			message(FATAL_ERROR "ERROR: Unable to find any usable Qt5 installation. Please install Qt5 or manually set the CMAKE_PREFIX_PATH.")
		endif()

		message(STATUS "Using Qt ${BEST_QT_MAJOR}.${BEST_QT_MINOR}.${BEST_QT_PATCH} MSVC ${BEST_QT_MSVC_YEAR} from ${BEST_QT_DIRECTORY}")
		SET(Qt5_DIR "${BEST_QT_DIRECTORY}/lib/cmake/Qt5/")
		SET(Qt5Test_DIR "${BEST_QT_DIRECTORY}/lib/cmake/Qt5Test")
	
	endif()
endmacro()
