// make HidIn and HidMsg
HidIn hi;
HidMsg msg;

// open joystick 0, exit on fail
if( !hi.openJoystick( 0 ) ) me.exit();

// infinite event loop
while( true )
{
    // wait on HidIn as event
    hi => now;

    // messages received
    while( hi.recv( msg ) )
    {
        // print for now
        <<< msg.type, msg.which, msg.idata, msg.fdata >>>;
    }
}
