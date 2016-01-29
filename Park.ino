// -----------------------------------------------------------------------------------
// functions related to Parking the mount

// sets the park postion as the current position
boolean setPark() {
  if ((parkStatus==NotParked) && (trackingState!=TrackingMoveTo)) {
    lastTrackingState=trackingState;
    trackingState=TrackingNone;

    // don't worry about moving around: during parking pec is turned off and backlash is cleared (0) so that targetAxis1/Dec=posAxis1/Dec
    // this should handle getting us back to the home position for micro-step modes up to 256X

   // if sync anywhere is enabled use the corrected location
  #ifdef SYNC_ANYWHERE_ON
    long h=(((long)targetAxis1.part.m+IHS)/1024L)*1024L;
    long d=(((long)targetAxis2.part.m+IDS)/1024L)*1024L;
  #else
    long h=((long)targetAxis1.part.m/1024L)*1024L;
    long d=((long)targetAxis2.part.m/1024L)*1024L;
  #endif

    // store our position
    EEPROM_writeQuad(EE_posAxis1 ,(byte*)&h);
    EEPROM_writeQuad(EE_posAxis2,(byte*)&d);

    // and the align
    saveAlignModel();

    // and remember what side of the pier we're on
    EEPROM.write(EE_pierSide,pierSide);
    parkSaved=true;
    EEPROM.write(EE_parkSaved,parkSaved);

    trackingState=lastTrackingState;
    return true;
  }
  return false;
}

boolean saveAlignModel() {
  // and store our corrections
  EEPROM_writeQuad(EE_doCor,(byte*)&doCor);
  EEPROM_writeQuad(EE_pdCor,(byte*)&pdCor);
  EEPROM_writeQuad(EE_altCor,(byte*)&altCor);
  EEPROM_writeQuad(EE_azmCor,(byte*)&azmCor);
  EEPROM_writeQuad(EE_IH,(byte*)&IH);
  EEPROM_writeQuad(EE_ID,(byte*)&ID);
  return true;
}

// takes up backlash and returns to the current position
boolean parkClearBacklash() {

  // backlash takeup rate
  cli();
  long LasttimerRateAxis1 =timerRateAxis1;
  long LasttimerRateAxis2=timerRateAxis2;
  timerRateAxis1 =timerRateBacklashAxis1;
  timerRateAxis2=timerRateBacklashAxis2;
  sei();

  // figure out how long we'll have to wait for the backlash to clear (+50%)
  long t; if (backlashAxis1>backlashAxis2) t=((long)backlashAxis1*1500)/(long)StepsPerSecondAxis1; else t=((long)backlashAxis2*1500)/(long)StepsPerSecondAxis1;
  t=(t/BacklashTakeupRate+250)/12;

  // start by moving fully into the backlash
  cli();
  targetAxis1.part.m += backlashAxis1;
  targetAxis2.part.m += backlashAxis2;
  sei();

  // wait until done or timed out
  for (int i=0; i<12; i++) if ((blAxis1!=backlashAxis1) || (posAxis1!=(long)targetAxis1.part.m) || (blAxis2!=backlashAxis2) || (posAxis2!=(long)targetAxis2.part.m)) delay(t);

  // then reverse direction and take it all up
  cli();
  targetAxis1.part.m  -= backlashAxis1;
  targetAxis2.part.m -= backlashAxis2;
  sei();

  // wait until done or timed out, plus a safety margin
  for (int i=0; i<24; i++) if ((blAxis1!=0) || (posAxis1!=(long)targetAxis1.part.m) || (blAxis2!=0) || (posAxis2!=(long)targetAxis2.part.m)) delay(t);

  // we arrive back at the exact same position so ftargetAxis1/Dec don't need to be touched
  
  // move at the previous speed
  cli();
  timerRateAxis1 =LasttimerRateAxis1;
  timerRateAxis2=LasttimerRateAxis2;
  sei();
  
  // return true on success
  if ((blAxis1!=0) || (blAxis2!=0)) return false; else return true;
}

// moves the telescope to the park position, stops tracking
byte park() {
  // Gets park position and moves the mount there
  if (trackingState!=TrackingMoveTo) {
    parkSaved=EEPROM.read(EE_parkSaved);
    if (parkStatus==NotParked) {
      if (parkSaved) {
        // stop tracking
        abortTrackingState=trackingState;
        lastTrackingState=TrackingNone;
        trackingState=TrackingNone; 

        // turn off the PEC while we park
        disablePec();
        PECstatus=IgnorePEC;
  
        // record our status
        parkStatus=Parking;
        EEPROM.write(EE_parkStatus,parkStatus);
        
        // get the position we're supposed to park at
        long h; EEPROM_readQuad(EE_posAxis1,(byte*)&h);
        long d; EEPROM_readQuad(EE_posAxis2,(byte*)&d);
        
        // now, slew to this target HA,Dec
        byte gotoPierSide=EEPROM.read(EE_pierSide);

        // if sync anywhere is enabled we have a corrected location, convert to instrument
        // and make sure we land on full-step, and store this new location so we remember PEC
  #ifdef SYNC_ANYWHERE_ON
        h=((h-IHS)/1024L)*1024L;
        d=((d-IDS)/1024L)*1024L;
        // also save the alignment index values in this mode since they can change widely
        EEPROM_writeQuad(EE_IH,(byte*)&IH);
        EEPROM_writeQuad(EE_ID,(byte*)&ID);
  #endif

        goTo(h,d,h,d,gotoPierSide);

        return 0;
      } else return 1; // no park position saved
    } else return 2; // not parked
  } else return 3; // already moving
}

// returns a parked telescope to operation, you must set date and time before calling this.  it also
// depends on the latitude, longitude, and timeZone; but those are stored and recalled automatically
boolean unpark() {
  parkStatus=EEPROM.read(EE_parkStatus);
  parkSaved =EEPROM.read(EE_parkSaved);
  parkStatus=Parked;
  if (trackingState!=TrackingMoveTo) {
    if (parkStatus==Parked) {
      if (parkSaved) {
        // enable the stepper drivers
        digitalWrite(Axis1_EN,Axis1_Enabled);
        digitalWrite(Axis2_EN,Axis2_Enabled);
        delay(10);

        // get corrections
        EEPROM_readQuad(EE_doCor,(byte*)&doCor);
        EEPROM_readQuad(EE_pdCor,(byte*)&pdCor);
        EEPROM_readQuad(EE_altCor,(byte*)&altCor);
        EEPROM_readQuad(EE_azmCor,(byte*)&azmCor);
        EEPROM_readQuad(EE_IH,(byte*)&IH);
        IHS=IH*15.0*StepsPerDegreeAxis1;
        EEPROM_readQuad(EE_ID,(byte*)&ID);
        IDS=ID*StepsPerDegreeAxis2;

        // get our position
        cli();
        EEPROM_readQuad(EE_posAxis1,(byte*)&posAxis1);   targetAxis1.part.m=posAxis1; targetAxis1.part.f=0;
        EEPROM_readQuad(EE_posAxis2,(byte*)&posAxis2); targetAxis2.part.m=posAxis2; targetAxis2.part.f=0;

  // if sync anywhere is enabled we have a corrected location, convert to instrument
  // just like we did when we parked
  #ifdef SYNC_ANYWHERE_ON
        posAxis1=((posAxis1-IHS)/1024L)*1024L;
        posAxis2=((posAxis2-IDS)/1024L)*1024L;
  #endif
        sei();
  
        // see what side of the pier we're on
        pierSide=EEPROM.read(EE_pierSide);
        if (pierSide==PierSideWest) DecDir = DecDirWInit; else DecDir = DecDirEInit;

        // set Meridian Flip behaviour to match mount type
        #ifdef MOUNT_TYPE_GEM
        meridianFlip=MeridianFlipAlways;
        #else
        meridianFlip=MeridianFlipNever;
        #endif
        
        // update our status, we're not parked anymore
        parkStatus=NotParked;
        EEPROM.write(EE_parkStatus,parkStatus);
          
        // start tracking the sky
        trackingState=TrackingSidereal;
        
        return true;
      };
    };
  };
  return false;
}

