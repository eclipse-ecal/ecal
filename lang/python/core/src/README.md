# This is the eCAL python API


Below you have some general ideas on how to use the API and retrieve documentation

### eCALSys


        import ecal_sys_python
        import os
        
###### Usage of help -> module[.class[.function]]

        help(ecal_sys_python.eCALSys.restart_task_list)
        
###### Can also work with enums

        help(ecal_sys_python.eCAL_Process_eStartMode)

###### This for-loop lists each class in the module
###### Can also be used to show each function from a specified class

        for function in dir(ecal_sys_python):
            print function

###### Constructor / Initializer of eCALSys application or any other class

        sys_app = ecal_sys_python.eCALSys() 
        options = ecal_sys_python.Options()
        sys_app.set_options(options) # options are empty in this case
        sys_app.save_config(os.getcwd()+r"\config") #save configuration to file "config" under current working directory

###### Initializer of enum with value

        enum_example = ecal_sys_python.eCAL_Process_eStartMode.proc_smode_normal
        print enum_example

###### Adding values to a list
        sys_task = ecal_sys_python.eCALSysTask()

###### Always use such functions in order to make changes to local variabiles if possbile
        sys_app.add_task(sys_task)
        print len(sys_app.get_task_list()) 
###### This will not add the new intance to the list  
        sys_app.get_task_list().append(ecal_sys_python.eCALSysTask()) 
        task_list = sys_app.get_task_list()
        task_list.append(ecal_sys_python.eCALSysTask())
        print len(sys_app.get_task_list())
        print len(task_list)
        
Reason why this does not work its due to implementation of get_task_list which returns a copy of the list inside the object


#### Sample

        import ecal_sys_python
        import os
        sys_app = ecal_sys_python.eCALSys() 

        success = sys_app.load_config(r"C:\CTZS_Framework\bin\TestConfig.ecalsys")

        print "Current task list"
        print "Name\tID\tTarget"
        for task in sys_app.get_task_list():
          print task.get_name() + "\t"+str(task.get_id()) + "\t" + task.get_target()


        print "\nRunners"
        print "Name\tID"
        for runner in sys_app.get_runner_list():
          print runner.get_name() + "\t"+str(runner.get_id())

        new_task = ecal_sys_python.eCALSysTask()
        new_task.set_name("Task "+str(len(sys_app.get_task_list())+1))
        new_task.set_target(sys_app.get_task(1).get_target());

        runner = ecal_sys_python.eCALSysRunner(5,"Notepad Launcher", r"C:\Windows\notepad.exe","","");
        new_task.set_runner(runner) 

        sys_app.add_task(new_task) 
        sys_app.add_runner(runner)
        print "\nAdding runner"
        print "\nRunners"
        print "Name\tID"
        for runner in sys_app.get_runner_list():
          print runner.get_name() + "\t"+str(runner.get_id())

        print "\nConfiguration after adding a new task"
        print "Name\tID\tTarget"
        for task in sys_app.get_task_list():
          print task.get_name() + "\t"+str(task.get_id()) + "\t" + task.get_target()

        sys_app.get_task(len(sys_app.get_task_list())).start()

        sys_app.remove_runner(runner,False)
        print "\nRemoving the runnner"
        print "\nRunners"
        print "Name\tID"
        for runner in sys_app.get_runner_list():
          print runner.get_name() + "\t"+str(runner.get_id())


        print "Saving configuration"
        sys_app.save_config(r"C:\CTZS_Framework\bin\TestConfig.ecalsys")
        

In order to load a configuration we suggest that you open eCALSys application and save any configuration. 
For this sample it is for the best if you create at least one empty task and change the path to the configuration file accordingly.


### eCALConv

        import ecal_conv_python
        
        converter = ecal_conv_python.eCALConv()
        converter.get_importer_plugins();
        raw_input("Press Enter to continue...")
        converter.get_exporter_plugins();
        raw_input("Press Enter to continue...")
        converter.set_importer_name("HDF5 Importer")
        converter.set_input_path("D:\eCAL_TSR_MFCImage_rec\measurement_2018-04-17-16-50-13-466\TMDF831G\measurement_2018-04-17-16-50-13-466.hdf5")
        converter.set_exporter_name("HDF5 Exporter")
        converter.set_output_path("D:\eCAL_TSR_MFCImage_rec\measurement_2018-04-17-16-50-13-466\TMDF831fG")
        converter.set_quite_and_verbose(False,True)
        converter.set_absolute_timespan(True)
        converter.set_trim_timespan("1s:10s")
        converter.set_offset_time("0s")
        channels_name_map = list()
        channels_name_map.append("MFCImageRightPb:MFCImageLeftPb")
        converter.set_channel_name_mapping(channels_name_map)
        converter.set_exporter_options("splitsize = '1024'")

        
        converter.run()
    
    
