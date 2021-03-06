/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#define CONTROL_SWITCH_COUNTER  20  // 20 iterations at 100hz (i.e. 2/10th of a second) at a new switch position will cause flight mode change
static void read_control_switch()
{
    static uint8_t switch_counter = 0;

    uint8_t switchPosition = readSwitch();

    // has switch moved?
    // ignore flight mode changes if in failsafe
    if (oldSwitchPosition != switchPosition && !failsafe.radio && failsafe.radio_counter == 0) {
        switch_counter++;
        if(switch_counter >= CONTROL_SWITCH_COUNTER) {
            oldSwitchPosition       = switchPosition;
            switch_counter          = 0;

            // set flight mode and simple mode setting
            if (set_mode(flight_modes[switchPosition])) {

                if(g.ch7_option != AUX_SWITCH_SIMPLE_MODE && g.ch8_option != AUX_SWITCH_SIMPLE_MODE && g.ch7_option != AUX_SWITCH_SUPERSIMPLE_MODE && g.ch8_option != AUX_SWITCH_SUPERSIMPLE_MODE) {
                    // set Simple mode using stored paramters from Mission planner
                    // rather than by the control switch
                    if (BIT_IS_SET(g.super_simple, switchPosition)) {
                        set_simple_mode(2);
                    }else{
                        set_simple_mode(BIT_IS_SET(g.simple_modes, switchPosition));
                    }
                }
            }

        }
    }else{
        // reset switch_counter if there's been no change
        // we don't want 10 intermittant blips causing a flight mode change
        switch_counter = 0;
    }
}

static uint8_t readSwitch(void){
    int16_t pulsewidth = g.rc_5.radio_in;   // default for Arducopter

    if (pulsewidth < 1231) return 0;
    if (pulsewidth < 1361) return 1;
    if (pulsewidth < 1491) return 2;
    if (pulsewidth < 1621) return 3;
    if (pulsewidth < 1750) return 4;        // Software Manual
    return 4;                               // Hardware Manual
}

static void reset_control_switch()
{
    oldSwitchPosition = -1;
    read_control_switch();
}

// read_3pos_switch
static uint8_t read_3pos_switch(int16_t radio_in){
    if (radio_in < AUX_SWITCH_PWM_TRIGGER_LOW) return AUX_SWITCH_LOW;      // switch is in low position
    if (radio_in > AUX_SWITCH_PWM_TRIGGER_HIGH) return AUX_SWITCH_HIGH;    // switch is in high position
    return AUX_SWITCH_MIDDLE;                                       // switch is in middle position
}

// read_aux_switches - checks aux switch positions and invokes configured actions
static void read_aux_switches()
{
    uint8_t switch_position;

    // exit immediately during radio failsafe
    if (failsafe.radio || failsafe.radio_counter != 0) {
        return;
    }

    // check if ch7 switch has changed position
    switch_position = read_3pos_switch(g.rc_7.radio_in);
    if (ap.CH7_flag != switch_position) {
        // set the CH7 flag
        ap.CH7_flag = switch_position;

        // invoke the appropriate function
        do_aux_switch_function(g.ch7_option, ap.CH7_flag);
    }

    // check if Ch8 switch has changed position
    switch_position = read_3pos_switch(g.rc_8.radio_in);
    if (ap.CH8_flag != switch_position) {
        // set the CH8 flag
        ap.CH8_flag = switch_position;

        // invoke the appropriate function
        do_aux_switch_function(g.ch8_option, ap.CH8_flag);
    }
}

// init_aux_switches - invoke configured actions at start-up for aux function where it is safe to do so
static void init_aux_switches()
{
    // set the CH7 flag
    ap.CH7_flag = read_3pos_switch(g.rc_7.radio_in);
    ap.CH8_flag = read_3pos_switch(g.rc_8.radio_in);

    // init channel 7 options
    switch(g.ch7_option) {
        case AUX_SWITCH_SIMPLE_MODE:
        case AUX_SWITCH_SONAR:
        case AUX_SWITCH_FENCE:
        case AUX_SWITCH_RESETTOARMEDYAW:
        case AUX_SWITCH_SUPERSIMPLE_MODE:
        case AUX_SWITCH_ACRO_TRAINER:
        case AUX_SWITCH_EPM:
        case AUX_SWITCH_SPRAYER:
            do_aux_switch_function(g.ch7_option, ap.CH7_flag);
            break;
    }
    // init channel 8 option
    switch(g.ch8_option) {
        case AUX_SWITCH_SIMPLE_MODE:
        case AUX_SWITCH_SONAR:
        case AUX_SWITCH_FENCE:
        case AUX_SWITCH_RESETTOARMEDYAW:
        case AUX_SWITCH_SUPERSIMPLE_MODE:
        case AUX_SWITCH_ACRO_TRAINER:
        case AUX_SWITCH_EPM:
        case AUX_SWITCH_SPRAYER:
            do_aux_switch_function(g.ch8_option, ap.CH8_flag);
            break;
    }
}

// do_aux_switch_function - implement the function invoked by the ch7 or ch8 switch
static void do_aux_switch_function(int8_t ch_function, uint8_t ch_flag)
{
    int8_t tmp_function = ch_function;

    // multi mode check
    if(ch_function == AUX_SWITCH_MULTI_MODE) {
        if (g.rc_6.radio_in < CH6_PWM_TRIGGER_LOW) {
            tmp_function = AUX_SWITCH_FLIP;
        }else if (g.rc_6.radio_in > CH6_PWM_TRIGGER_HIGH) {
            tmp_function = AUX_SWITCH_SAVE_WP;
        }else{
            tmp_function = AUX_SWITCH_RTL;
        }
    }

    switch(tmp_function) {
        case AUX_SWITCH_FLIP:
            // flip if switch is on, positive throttle and we're actually flying
            if((ch_flag == AUX_SWITCH_HIGH) && (g.rc_3.control_in >= 0) && ap.takeoff_complete) {
                init_flip();
            }
            break;

        case AUX_SWITCH_SIMPLE_MODE:
            // low = simple mode off, middle or high position turns simple mode on
            set_simple_mode(ch_flag == AUX_SWITCH_HIGH || ch_flag == AUX_SWITCH_MIDDLE);
            break;

        case AUX_SWITCH_SUPERSIMPLE_MODE:
            // low = simple mode off, middle = simple mode, high = super simple mode
            set_simple_mode(ch_flag);
            break;

        case AUX_SWITCH_RTL:
            if (ch_flag == AUX_SWITCH_HIGH) {
                // engage RTL (if not possible we remain in current flight mode)
                set_mode(RTL);
            }else{
                // return to flight mode switch's flight mode if we are currently in RTL
                if (control_mode == RTL) {
                    reset_control_switch();
                }
            }
            break;

        case AUX_SWITCH_SAVE_TRIM:
            if ((ch_flag == AUX_SWITCH_HIGH) && (control_mode <= ACRO) && (g.rc_3.control_in == 0)) {
                save_trim();
            }
            break;

        case AUX_SWITCH_SAVE_WP:
            // save waypoint when switch is brought high
            if (ch_flag == AUX_SWITCH_HIGH) {

                // if in auto mode, reset the mission
                if(control_mode == AUTO) {
                    aux_switch_wp_index = 0;
                    g.command_total.set_and_save(1);
                    set_mode(RTL);  // if by chance we are unable to switch to RTL we just stay in AUTO and hope the GPS failsafe will take-over
                    Log_Write_Event(DATA_SAVEWP_CLEAR_MISSION_RTL);
                    return;
                }

				// we're on the ground
				if((g.rc_3.control_in == 0) && (aux_switch_wp_index == 0)){
					// nothing to do
					return;
				}

                // initialise new waypoint to current location
                Location new_wp;

                if(aux_switch_wp_index == 0) {
                    // this is our first WP, let's save WP 1 as a takeoff
                    // increment index to WP index of 1 (home is stored at 0)
                    aux_switch_wp_index = 1;

                    // set our location ID to 16, MAV_CMD_NAV_WAYPOINT
                    new_wp.id = MAV_CMD_NAV_TAKEOFF;
                    new_wp.options = 0;
                    new_wp.p1 = 0;
                    new_wp.lat = 0;
                    new_wp.lng = 0;
                    new_wp.alt = max(current_loc.alt,100);

                    // save command:
                    // we use the current altitude to be the target for takeoff.
                    // only altitude will matter to the AP mission script for takeoff.
                    // If we are above the altitude, we will skip the command.
                    set_cmd_with_index(new_wp, aux_switch_wp_index);
                }

                // initialise new waypoint to current location
                new_wp = current_loc;

                // increment index
                aux_switch_wp_index++;

                // set the next_WP (home is stored at 0)
                // max out at 100 since I think we need to stay under the EEPROM limit
                aux_switch_wp_index = constrain_int16(aux_switch_wp_index, 1, 100);

                if(g.rc_3.control_in > 0) {
                    // set our location ID to 16, MAV_CMD_NAV_WAYPOINT
                    new_wp.id = MAV_CMD_NAV_WAYPOINT;
                }else{
					// set our location ID to 21, MAV_CMD_NAV_LAND
					new_wp.id = MAV_CMD_NAV_LAND;
                }

                // save command
                set_cmd_with_index(new_wp, aux_switch_wp_index);

                // log event
                Log_Write_Event(DATA_SAVEWP_ADD_WP);
            }
            break;

#if CAMERA == ENABLED
        case AUX_SWITCH_CAMERA_TRIGGER:
            if (ch_flag == AUX_SWITCH_HIGH) {
                do_take_picture();
            }
            break;
#endif

        case AUX_SWITCH_SONAR:
            // enable or disable the sonar
            if (ch_flag == AUX_SWITCH_HIGH) {
                g.sonar_enabled = true;
            }else{
                g.sonar_enabled = false;
            }
            break;

#if AC_FENCE == ENABLED
        case AUX_SWITCH_FENCE:
            // enable or disable the fence
            if (ch_flag == AUX_SWITCH_HIGH) {
                fence.enable(true);
                Log_Write_Event(DATA_FENCE_ENABLE);
            }else{
                fence.enable(false);
                Log_Write_Event(DATA_FENCE_DISABLE);
            }
            break;
#endif
        case AUX_SWITCH_RESETTOARMEDYAW:
            if (ch_flag == AUX_SWITCH_HIGH) {
                set_yaw_mode(YAW_RESETTOARMEDYAW);
            }else{
                set_yaw_mode(YAW_HOLD);
            }
            break;

        case AUX_SWITCH_ACRO_TRAINER:
            switch(ch_flag) {
                case AUX_SWITCH_LOW:
                    g.acro_trainer = ACRO_TRAINER_DISABLED;
                    Log_Write_Event(DATA_ACRO_TRAINER_DISABLED);
                    break;
                case AUX_SWITCH_MIDDLE:
                    g.acro_trainer = ACRO_TRAINER_LEVELING;
                    Log_Write_Event(DATA_ACRO_TRAINER_LEVELING);
                    break;
                case AUX_SWITCH_HIGH:
                    g.acro_trainer = ACRO_TRAINER_LIMITED;
                    Log_Write_Event(DATA_ACRO_TRAINER_LIMITED);
                    break;
            }
            break;
#if EPM_ENABLED == ENABLED
        case AUX_SWITCH_EPM:
            switch(ch_flag) {
                case AUX_SWITCH_LOW:
                    epm.off();
                    Log_Write_Event(DATA_EPM_OFF);
                    break;
                case AUX_SWITCH_MIDDLE:
                    epm.neutral();
                    Log_Write_Event(DATA_EPM_NEUTRAL);
                    break;
                case AUX_SWITCH_HIGH:
                    epm.on();
                    Log_Write_Event(DATA_EPM_ON);
                    break;
            }
            break;
#endif
#if SPRAYER == ENABLED
        case AUX_SWITCH_SPRAYER:
            sprayer.enable(ch_flag == AUX_SWITCH_HIGH);
            // if we are disarmed the pilot must want to test the pump
            sprayer.test_pump((ch_flag == AUX_SWITCH_HIGH) && !motors.armed());
            break;
#endif

        case AUX_SWITCH_AUTO:
            if (ch_flag == AUX_SWITCH_HIGH) {
                set_mode(AUTO);
            }else{
                // return to flight mode switch's flight mode if we are currently in AUTO
                if (control_mode == AUTO) {
                    reset_control_switch();
                }
            }
            break;

#if AUTOTUNE == ENABLED
        case AUX_SWITCH_AUTOTUNE:
            // turn on auto tuner
            switch(ch_flag) {
                case AUX_SWITCH_LOW:
                case AUX_SWITCH_MIDDLE:
                    // turn off tuning and return to standard pids
                    if (roll_pitch_mode == ROLL_PITCH_AUTOTUNE) {
                        set_roll_pitch_mode(ROLL_PITCH_STABLE);
                    }
                    break;
                case AUX_SWITCH_HIGH:
                    // start an auto tuning session
                    // set roll-pitch mode to our special auto tuning stabilize roll-pitch mode
                    set_roll_pitch_mode(ROLL_PITCH_AUTOTUNE);
                    break;
            }
            break;
#endif

        case AUX_SWITCH_LAND:
            if (ch_flag == AUX_SWITCH_HIGH) {
                set_mode(LAND);
            }else{
                // return to flight mode switch's flight mode if we are currently in LAND
                if (control_mode == LAND) {
                    reset_control_switch();
                }
            }
            break;
    }
}

// save_trim - adds roll and pitch trims from the radio to ahrs
static void save_trim()
{
    // save roll and pitch trim
    float roll_trim = ToRad((float)g.rc_1.control_in/100.0f);
    float pitch_trim = ToRad((float)g.rc_2.control_in/100.0f);
    ahrs.add_trim(roll_trim, pitch_trim);
    Log_Write_Event(DATA_SAVE_TRIM);
    gcs_send_text_P(SEVERITY_HIGH, PSTR("Trim saved"));
}

// auto_trim - slightly adjusts the ahrs.roll_trim and ahrs.pitch_trim towards the current stick positions
// meant to be called continuously while the pilot attempts to keep the copter level
static void auto_trim()
{
    if(auto_trim_counter > 0) {
        auto_trim_counter--;

        // flash the leds
        AP_Notify::flags.save_trim = true;

        // calculate roll trim adjustment
        float roll_trim_adjustment = ToRad((float)g.rc_1.control_in / 4000.0f);

        // calculate pitch trim adjustment
        float pitch_trim_adjustment = ToRad((float)g.rc_2.control_in / 4000.0f);

        // make sure accelerometer values impact attitude quickly
        ahrs.set_fast_gains(true);

        // add trim to ahrs object
        // save to eeprom on last iteration
        ahrs.add_trim(roll_trim_adjustment, pitch_trim_adjustment, (auto_trim_counter == 0));

        // on last iteration restore leds and accel gains to normal
        if(auto_trim_counter == 0) {
            ahrs.set_fast_gains(false);
            AP_Notify::flags.save_trim = false;
        }
    }
}

