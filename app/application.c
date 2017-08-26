#include <application.h>


bc_led_t led;

static void radio_event_handler(bc_radio_event_t event, void *event_param);

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

uint16_t event_counter = 0;
uint16_t lost_counter = 0;

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_radio_init();
    bc_radio_set_event_handler(radio_event_handler, NULL);
    bc_radio_listen();

    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_module_lcd_init(&_bc_module_lcd_framebuffer);

    static bc_button_t lcd_left;
    bc_button_init_virtual(&lcd_left, BC_MODULE_LCD_BUTTON_LEFT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_left, lcd_button_event_handler, NULL);

    static bc_button_t lcd_right;
    bc_button_init_virtual(&lcd_right, BC_MODULE_LCD_BUTTON_RIGHT, bc_module_lcd_get_button_driver(), false);
    bc_button_set_event_handler(&lcd_right, lcd_button_event_handler, NULL);

}

void application_task(void)
{
    /*if (!bc_module_lcd_is_ready())
    {
        return;
    }*/

    bc_module_core_pll_enable();

    int w;
    char str[32];

    bc_module_lcd_clear();


    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(4, 4, "bcf-rf-test-rx", true);

    bc_module_lcd_set_font(&bc_font_ubuntu_28);
    snprintf(str, sizeof(str), "%d", event_counter);
    w = bc_module_lcd_draw_string(10, 25, str, true);

    snprintf(str, sizeof(str), "%d", lost_counter);
    w = bc_module_lcd_draw_string(10, 60, str, true);


    bc_module_lcd_update();

    bc_module_core_pll_disable();

    //bc_scheduler_plan_current_relative(500);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enrollment_start();
        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}

void lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) event_param;

    if (event != BC_BUTTON_EVENT_CLICK)
    {
        return;
    }

    if (self->_channel.virtual_channel == BC_MODULE_LCD_BUTTON_LEFT)
    {
        lost_counter = 0;
        bc_scheduler_plan_now(0);
    }
    else
    {

    }

    bc_scheduler_plan_now(0);
}

void bc_radio_on_push_button(uint64_t *peer_device_address, uint16_t *event_count)
{
    lost_counter += *event_count - (event_counter + 1);
    event_counter = *event_count;

    bc_led_pulse(&led, 100);

    bc_scheduler_plan_now(0);
}


static void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_DETACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_INIT_DONE)
    {
        //my_device_address = bc_radio_get_device_address();
    }
}

