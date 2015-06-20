void sleepNow()
{
	if(1 == canSleep)
	{
		/*	Power Reduction Register Timer Note
			Disabling the 8 bit timer from the Power Reduction Register
			otherwise it'll trigger and we won't be able to go into sleep
			because the Arduino platform uses it for the millis().
			We don't need it for counting time elapsed since boot so it's okay.
		*/
		//	PRR = PRR | 0b11100000;
		/*	Sleep Mode Note
			Setting sleep mode to idle only because the button presses and the 
			clocks rely on pin change interrupts
		*/
		#ifdef DEBUG
			digitalWrite(LEDPIN,LOW);
		#endif
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		sleep_mode();
		sleep_disable();
		#ifdef DEBUG
			digitalWrite(LEDPIN,HIGH);
		#endif
	}
}