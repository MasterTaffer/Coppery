# Coppery demo game 

This directory contains the data required by the simple demo game. Compile 
Coppery engine and toss the binary into this folder and run.

## How to do stuff?

Check the data/angelscript directory for the game scripts. All AngelScript
code is visible from all other files: there is no module system in place.

Other good place to check for cool stuff is data/control, where the control
scripts chill. The control scripts are used for much lower level functionality,
loading shaders, defining graphics pipeline etc.

## AngelScript tests

The data/angelscript/tests contain a few tests for some subsystems. Check them
out, if you care that is. They provide some insight in the functionality that
Coppery engine provides.

    ./Coppery --control-run 'ScriptEngine.RunTests("TEST-asunit.xml")'
    
runs the tests and outputs the results to TEST-asunit.xml in (somewhat 
compliant) JUnit xml format.

## Directory overview

**config**

Contains a few configuration files for graphics, input, etc.
input.cfg contains all the input definitions. Is bound to virtual path
"user/config"

**static**

Static engine data. Is bound to virtual path "static".

**userdata**

The user data folder. The engine has read write access to this folder. Is
bound to virtual path "user/data"

**data/assets**

All graphical assets required by the game. Is bound to virtual path "assets"

**data/assets/def**

Contains asset definitions. These are parsed by the engine. Assets can also
be defined in the Control Lua scripts (see data/control).
Is bound to virtual path "control".

**data/control**

Control Lua scripts. At the engine start, init.lua is executed. There are
many functions and interfaces exposed to the Lua. poststart.lua is executed
after all engine components have been initialized.

**data/angelscript**

AngelScript scripts reside here. Is bound to virtual path "angelscript"

If the engine variable Compile.Scripts is 1 or above,
these scripts are compiled at the engine startup. The compilation
result is then stored to the 

**data/compiled**

Contains compiled AngelScript bytecode. Is bound to virtual path "compiled"

### Directory redirection

The engine has multiple command line flags for changing the paths above.

**--working-dir**

Sets the application base directory. Note that this doesn't actually change
the working directory of the application: info and error logs still get output
to the real working directory.

Default value:
    
    --working-dir ./

**--data**

Path to the "data" folder.

Default value:

    --data ./data

**--dir-control --dir-angelscript --dir-compiled --dir-static --dir-user --dir-config --dir-user-data**

Paths to the corresponding subsystem directories.





