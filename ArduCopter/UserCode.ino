/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

AP_Relay relays;

#ifdef USERHOOK_INIT
void userhook_init()
{
    // put your initialisation code here
    // this will be called once at start-up
    relays.init();
    relays.off(0);
    relays.off(1);
    
    relays.off(2);
    
    //initialize rc_10
    //g.rc_10.function = 1;
}
#endif

#ifdef USERHOOK_FASTLOOP
void userhook_FastLoop()
{
    // put your 100Hz code here
}
#endif

#ifdef USERHOOK_50HZLOOP
void userhook_50Hz()
{
    // put your 50Hz code here
}
#endif

#ifdef USERHOOK_MEDIUMLOOP
void userhook_MediumLoop()
{
    // put your 10Hz code here

}
#endif

#ifdef USERHOOK_SLOWLOOP
void userhook_SlowLoop()
{
    // put your 3.3Hz code here
    //cliSerial->printf("Slow Loop");
    //check control_mode variable found in arducopter file
    if(control_mode == ARCQ) {
      //set servo to 90 degrees
      //g.rc_10.set_radio_trim(RC_Channel_aux::k_manual);
      relays.off(2);
      
      if(g.rc_6.control_in < 250) {
        relays.on(0);
        relays.off(1);
      }
      else if(g.rc_6.control_in > 750){
        relays.off(0);
        relays.on(1);
      }
      else{
        relays.off(0);
        relays.off(1);
      }      
    }
    else {
      relays.off(0);
      relays.off(1);
      //set servo to 0 degrees
      //g.rc_10.set_radio_to_min(RC_Channel_aux::k_manual);
      relays.on(2);
    }
}
#endif

#ifdef USERHOOK_SUPERSLOWLOOP
void userhook_SuperSlowLoop()
{
    // put your 1Hz code here
}
#endif
