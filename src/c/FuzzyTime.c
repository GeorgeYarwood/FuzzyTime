#include <pebble.h>
#include <stdio.h>
static Window* s_window;
static TextLayer* hr_prefix_text_layer;
static TextLayer* min_prefix_text_layer;
static TextLayer* hr_text_layer;

char* hrPrefixBuf;
char* minPrefixBuf;
char* hrBuf;

static void update_time(struct tm* tick_time);

static bool isFuzzy = true;

#define BUF_SIZE 32

static void update_time_now()
{
	time_t temp = time(NULL);
	struct tm* tick_time = localtime(&temp);
	update_time(tick_time);
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void* context)
{
	isFuzzy = false;
	update_time_now();
}

static void prv_select_click_handler_release(ClickRecognizerRef recognizer, void* context)
{
	isFuzzy = true;
	update_time_now();
}

static void prv_click_config_provider(void* context)
{
	window_long_click_subscribe(BUTTON_ID_BACK, 700, prv_select_click_handler, prv_select_click_handler_release);
}

static void prv_window_load(Window* window)
{
	Layer* window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	hr_prefix_text_layer = text_layer_create(GRect(0, 2, bounds.size.w, 50));
	min_prefix_text_layer = text_layer_create(GRect(0, 20, bounds.size.w, 50));
	hr_text_layer = text_layer_create(GRect(0, 50, bounds.size.w, 125));

	text_layer_set_background_color(hr_prefix_text_layer, GColorWhite);
	text_layer_set_background_color(min_prefix_text_layer, GColorClear);
	text_layer_set_background_color(hr_text_layer, GColorBlack);


	text_layer_set_text_color(hr_prefix_text_layer, GColorBlack);
	text_layer_set_text_color(min_prefix_text_layer, GColorBlack);
	text_layer_set_text_color(hr_text_layer, GColorWhite);

	text_layer_set_font(hr_prefix_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	text_layer_set_font(min_prefix_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_font(hr_text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));

	text_layer_set_text_alignment(hr_prefix_text_layer, GTextAlignmentCenter);
	text_layer_set_text_alignment(min_prefix_text_layer, GTextAlignmentCenter);
	text_layer_set_text_alignment(hr_text_layer, GTextAlignmentCenter);

	layer_add_child(window_layer, text_layer_get_layer(hr_prefix_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(min_prefix_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(hr_text_layer));

	update_time_now();
}

static void prv_window_unload(Window* window)
{
	text_layer_destroy(hr_prefix_text_layer);
	text_layer_destroy(min_prefix_text_layer);
	text_layer_destroy(hr_text_layer);
}

static void update_time(struct tm* tick_time)
{
	if(isFuzzy)
	{
		int min = tick_time->tm_min;
		int hr = tick_time->tm_hour >= 13 ? tick_time->tm_hour - 12 : tick_time->tm_hour;

		if (min <= 10)
		{
			snprintf(hrPrefixBuf, BUF_SIZE, "Just gone");
			snprintf(hrBuf, BUF_SIZE, "%i", hr == 0 ? 12 : hr);

			text_layer_set_font(hr_prefix_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
			memset(minPrefixBuf, 0, BUF_SIZE);
		}
		else if (min <= 45)
		{
			//char minPrfxBuf[20];
			snprintf(hrPrefixBuf, BUF_SIZE, "Around");

			bool zeroFix = true;

			//Round to 15, 30, 45
			if (min < 25)
			{
				snprintf(minPrefixBuf, BUF_SIZE, "Quarter past");
			}
			else if (min <= 40)
			{
				snprintf(minPrefixBuf, BUF_SIZE, "Half");
			}
			else
			{
				snprintf(minPrefixBuf, BUF_SIZE, "Quarter to");
				++hr;
				zeroFix = false;
			}

			text_layer_set_font(hr_prefix_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));

			snprintf(hrBuf, BUF_SIZE, "%i", (zeroFix && hr == 0) ? 12 : hr);
		}
		else
		{
			snprintf(hrPrefixBuf, BUF_SIZE, "Nearly");
			++hr;

			memset(minPrefixBuf, 0, BUF_SIZE);

			snprintf(hrBuf, BUF_SIZE, "%i", hr);
			text_layer_set_font(hr_prefix_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

		}
	}	
	else
	{
		int min = tick_time->tm_min;
		int hr = clock_is_24h_style() ? tick_time->tm_hour :
			(tick_time->tm_hour >= 13 ? tick_time->tm_hour - 12 : tick_time->tm_hour);

		//Place preceeding 0 if required
		char tmphr[4];
		snprintf(tmphr, 4, "%i", hr);
		if (hr < 10)
		{
			tmphr[1] = tmphr[0];
			tmphr[0] = '0';
		}

		char tmpMins[4];
		snprintf(tmpMins, 4, "%i", min);
		if (min < 10)
		{
			tmpMins[1] = tmpMins[0];
			tmpMins[0] = '0';
		}

		snprintf(hrBuf, BUF_SIZE, "%s:%s", tmphr, tmpMins);
		memset(hrPrefixBuf, 0, BUF_SIZE);
		memset(minPrefixBuf, 0, BUF_SIZE);
	}

	text_layer_set_text(hr_prefix_text_layer, hrPrefixBuf);
	text_layer_set_text(min_prefix_text_layer, minPrefixBuf);
	text_layer_set_text(hr_text_layer, hrBuf);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed)
{
	update_time(tick_time);
}

static void prv_init(void)
{
	hrPrefixBuf = malloc(BUF_SIZE);
	minPrefixBuf = malloc(BUF_SIZE);
	hrBuf = malloc(BUF_SIZE);

	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	s_window = window_create();
	window_set_click_config_provider(s_window, prv_click_config_provider);
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = prv_window_load,
			.unload = prv_window_unload,
	});
	const bool animated = true;
	window_stack_push(s_window, animated);
}

static void prv_deinit(void)
{
	window_destroy(s_window);
	if (hrPrefixBuf)
	{
		free(hrPrefixBuf);
	}
	if (minPrefixBuf)
	{
		free(minPrefixBuf);
	}
	if (hrBuf)
	{
		free(hrBuf);
	}
}

int main(void)
{
	prv_init();

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

	app_event_loop();
	prv_deinit();
}
