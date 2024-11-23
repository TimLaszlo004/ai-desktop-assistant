# Desktop application to assist you with anything you want help with
This is a highly customizabe, real-time extendable AI assistant application. It leverages OpenAI's function calling along with Whisper (through OpenAI API), Google's WebRTCVAD and text-to-speech models.
Functions can be defined, which will then be executable on your machine! You can add, edit and remove functions as you like, real-time. The whole applicaation can be used through a GUI made with Qt.



## How does it work
The application was made with modules:
 - listener (uses mic to record audio with speech in it)
 - transcriber (transcribes audio file)
 - sender (sends message - receives message)
 - reader (reads message text response)
 - actuator (executes message function response)
 - trigger (+storage) (manages all the modules above)

Each module can be replaced easily without understanding the whole application, if you follow some non-hindering rules.



## How to use
There are two very crucial JSON files: `commands.json` and `map.json`, which you should check before running the app.
The first one stores the modules info, which is:
 - **how to call a module:** This is a shell command, you can also execute it via cmd or PowerShell. Each module parses the arguments and sets up the variables.
 - **when to call a module:** A file path can be specified with a filename and extension. If a new file appers in the storage (more on that later) it will run the module, with replacing the "{}" with the filename. You can also use {data:value:othervalue} format if the file is JSON and only a text from the JSON should be used to replace.

A `global.json` was added to be able to use global variables, such as API keys and directories in the `commands.json` file. You should provide an OpenAI API key and the directory where the app is located (this might be unnecessary soon).

The `map.json` specifies the callable functions. At the end of the day each function is a command, which can be executed in the OS specific shell. You can specify these commands yourself if you are comfortable with it or use the `typer.py` script to imitate keyboard actions. If you use the GUI, you can also do this from a friendly interface.


#### You can use it from the GUI:
Just start up assistant.exe and you will be able to edit the configurable files, see the conversation history, use text input and of course start and stop the Assistant.


#### Or use it headless:
The GUI is taking care of a bunch of things, but it can run without it. You can start the trigger module, the listener module (if you want mic) and the storage.
The storage manages the file handling: each module produces some kind of file, except **reader** and **actuator**, but instead of writing it on disk and reading it back again from another module, a "storage" is used, to store it in RAM and access it from anywhere on your machine. This was implemented as a Flask server, which is listening on localhost:5000, so you can easily build any GUI or application upon this base.



## Other plaforms
During the development every solution and library was picked with multiple platforms in mind. Currently it does not support MacOS or Linux systems but it is easy to migrate and build on other platforms.



## Side note
This project is a University project and thus, the time for implementation was limited. I always kept in mind, that any programmer should be able to easily replace modules, but this was not properly tested, although the few test was successful. OpenAI function calling is not unique, you can change to any LLM API you like. Also, recently the OpenAI text-to-text API is available, which makes the transcriber and reader module unnecessary, but this is so fresh I did not use it due to the time limitations. However it can be easily changed: remove the transcriber and reader modul and rewrite the sender module. Then update `commands.json` and set the trigger info to what transcribers used to be.
For building your application use pyinstaller and/or nuitka, as I als did.
