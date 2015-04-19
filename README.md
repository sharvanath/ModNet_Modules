ModNet Modules.<br>
For details about the ModNet project refer http://sharvanath.info/modnet/

This reporsitory has ModNet user level modules, and some utilities. This Project is under active development, so feel free contact me at sharvanath@gmail.com if you have any questions. Right now we I haven't added a configure script for installing the dependencies, but it will be added soon. The repository only has the code for the dummy module right now, codes for the gzip and jpeg compression modules, ssd swap modules and deduplication modules will be added soon. In the meantime if you would like to have the code for them contact me at sharvanath@gmail.com.<br>

The directory stucture is:<br>
ModNet_Modules/include:     The header files for module and utils.<br>
ModNet_Modules/utils:       Contains code for the apply_module binary, which forks a new process for the given binary and                                   applied the specified list of modules. Execute the binary for usage instructions.<br>

ModNet_Modules/dummy_module: Code for the dummy module. Execute the binary for usage instructions.<br>
ModNet_Modules/dummy_module_lib: Code for the dummy module using the new library (modnet_main). A module can be defined by defining the modnet_module_operations structure. 


