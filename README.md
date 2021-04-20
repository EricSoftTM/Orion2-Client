# Orion2-Client
Orion2 - A MapleStory2 Dynamic Link Localhost

----------------------------------------------------------------------
## Setting Up
To use the Orion2-Client DLL Localhost, we must be able to inject it into the MapleStory2 Client.
In order to do this, we will need to modify the import table of a dependency and add Orion2 exports. 

 1) Download and install [CFF Explorer](http://google.com), open the program.
 2) File->Open, and browse to your MS2 directory - select/open NxCharacter.dll.
 3) Tab into 'Import Adder' and select 'Add'. Browse for the Orion2.dll and open it. 
 4) After selecting the DLL, exports will appear below. Highlight them all and click 'Import By Name'.
 5) Select 'Rebuild Import Table' and click OK once done. 
 6) You're done. File->Save As the file as NxCharacter.dll (make a backup if necessary).

By default, the client will connect to localhost (127.0.0.1). 
To configure a custom IP, simply create a new file in your game directory called `Orion.ini`. 
Within the file, configure the following where `127.0.0.1` is the IP you want to connect to:
```
[Settings]
ClientIP=127.0.0.1
```

----------------------------------------------------------------------
 ## Client Documentation
First, take note that MS2 clients up to v9 (latest), the following applies:
 * The client does **NOT** contain IP Checks
 * The client does **NOT** contain CRC Checks
 * The client accepts custom *IP* and *Port* arguments
 * The original clients contain Page Protection and will crash from standard memory alteration
 
The MapleStory2 clients respond to the following command line arguments:
 * If an argument contains 'CRASHDUMP:/', startup will exit and return false - this sends crash report info.
 * If the application contains exactly 3 args, it will parse them as so (ignores loading internal IPs):
    * arg[0] = Target Application
    * arg[1] = Gateway IP
    * arg[2] = Gateway Port
 * If the command line contains the above 3 arguments, it will begin to parse the optional args:
    * -id
    * -season
    * -env
    * -locale
    * -language
    * -ignorePack
----------------------------------------------------------------------
## Client Redirection
The MapleStory2 client can still utilize (and is included in Orion2) the WinSock hook to redirect from Nexon server IP's to your own. However, you'll notice that regardless of the hook, your client won't physically launch: this is because the client relies on responses from the Nexon Launcher. 

So, how do we bypass these NXL checks to execute the client as our localhost?
 * Bypass the `IsNXLEnabled` check in the `CGameApp::ParseCmdArgs` function.
    * This enables the *IP* and *Port* args from above. This may be replaced by using WinSock hooks.
 * Bypass the `IsNXLEnabled` check in the `CGameApp::InitInstance` function.
    * Without bypassing this, your client will either infinitely hang or close (never opens). This one **MUST BE BYPASSED**!

Additional redirection bypasses that are possible:
  * Auto-Login from NXL: This needs a conditional in the ConnectGateway func. to be bypassed.
      * This will enable the Username/Password box on the login screen.
      * To receive account info and not passports, you need to change it in CRequest::OnRequestLogin.
  * Auto-Migrate: This needs a conditional call to be bypassed in the world select rendering function.
      * This will enable the World Selection screen, displaying connectable worlds and their user counts.
----------------------------------------------------------------------
## Client Hacks
Just like MapleStory, there are various client hacks that can be applied to make things possible that Nexon wouldn't. The below client hacks will all be included in the Orion2 Localhost Client.

### Enable Swear Filter
To bypass Nexon's meme swear filter, find the string "banWord.xml" and xref it. It'll xref to a StringPool parsing function. 

Trace it to the function and it should look something like this: 
```
  MEMORY[0x78131F51](&v8, "banWord.xml");
  v5 = (int)(v1 + 92);
  v4 = (int)(v1 + 92);
  v6 = &v4;
  v10 = 2;
  sub_596940(&v4, &v8);
  sub_44B430(v4, v5); <--
 ```
Simply **NOP** the `sub_44B430` function, and the swear filter will be disabled.

### Enable Infinite Chat Text
Still working on implementing this.

### Enable Chat Spam Bypass
Still working on implementing this.

### Enable Droppable NX
Still working on implementing this.

### Enable Multi-Client
Still working on implementing this.
