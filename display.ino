void clock_page() {
	const uint8_t clockpos_x = 2, clockpos_y = 45;// clockpos is the x-axis pixel position
	uint8_t timebar = 0;
	u8g.firstPage();
	do {
		DS3231_get(&time);
		char time_string[BUFF_MAX];
		snprintf(time_string, BUFF_MAX, "%02d:%02d", time.hour, time.min);
		u8g.setFont(u8g_font_fur35n);
		u8g.drawStr(clockpos_x, clockpos_y, time_string);
		
		timebar = 2 * time.sec;
		u8g.drawBox(5, 50, timebar, 3);
	} while( u8g.nextPage() );
}