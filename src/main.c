/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"
#include "mgos_mqtt.h"
#include "mgos_gpio.h"
#include "mgos_http_server.h"
#include "mgos_dht.h"
#include "mgos_adc.h"


#define PIN_BOTON 0
#define LED_PIN 2
#define BOTON_PRESIONADO mgos_gpio_read(PIN_BOTON)==0
#define PIN_DHT 5
#define DESTELLO 50
#define PIN_LDR 0 // A0 es igual a 17
#define PIN_COOLER_1 14 // GPIO14 = D5
#define PIN_COOLER_2 12 // GPIO12 = D6
#define PIN_TIRA_LED 4 // GPIO04 = D2
#define ACTIVAR_COOLER_1 mgos_gpio_write(PIN_COOLER_1, true)
#define DESACTIVAR_COOLER1 mgos_gpio_write(PIN_COOLER_1, false)
#define CONFIGURAR_COOLER1 mgos_gpio_setup_output(PIN_COOLER_1, 0)

bool encendidoFijo=false;
int tiempoLed = 500;
mgos_timer_id id=NULL;
mgos_timer_id soltar=NULL;
mgos_timer_id tmpBomba=NULL;
mgos_timer_id apagarBoton=NULL;
mgos_timer_id timercb=NULL;
bool secuencia_bomba_en_accion=false;

int tiempoEspera=5000;
int tiempoPreaviso=1000;
bool comenzar=false;
int64_t tiempoInicio;
int contador=0;
float temp;
float hum;
int luz;




struct mgos_dht *dht;

static void cb_boton_pulsado(int pin, void *arg);
static void cb_boton_liberado(int pin, void *arg);
static void mostrar_pulsaciones(void *arg);


static void apagar_led(void *arg){
  mgos_gpio_write(LED_PIN, true);
  (void) arg;
}

static void fin_secuencia_bomba(void *arg){
  mgos_clear_timer(apagarBoton);
  mgos_clear_timer(timercb);
  mgos_gpio_write(LED_PIN, false);
  secuencia_bomba_en_accion=false;
  tiempoLed=500;
  (void) arg;
}



static void encender_led(void *arg)
    {
//    static int tiempoInicio= mgos_uptime();
  //  LOG(LL_INFO, tiempoInicio);
//    if(mog)

    mgos_gpio_write(LED_PIN, false);
    apagarBoton=mgos_set_timer(DESTELLO, 0, apagar_led, NULL);

    (void) arg;
    }


/*
static void foo_handler(struct mg_connection *c, int ev, void *p,
                                void *user_data) {
          (void) p;
          (void) c;
          (void) ev;
          tiempoLed=1000;
          (void) user_data;
        }

static void test(struct mg_connection *c, int ev, void *p,
                        void *user_data) {
      (void) p;
      if (ev != MG_EV_HTTP_REQUEST) return;
      LOG(LL_INFO, ("prueba"));
      tiempoLed=1000;
      (void) c;
      (void) user_data;

}

static void foo_handler(struct mg_connection *c, int ev, void *p,
                        void *user_data) {
  (void) p;
  if (ev != MG_EV_HTTP_REQUEST) return;
  LOG(LL_INFO, ("Foo requested"));
  mg_send_response_line(c, 200,
                        "Content-Type: text/html\r\n");
  //tiempoLed=1000;
  mg_printf(c, "<class='image fit'><img src='%s.jpg'/></a>", tiempoLed==100 ? "dos" : "uno");
  mg_printf(c, "<script type='text/javascript'>setTimeout('location.reload()', 1000);</script>");
  mg_printf(c, "<h5>El Tiempo de encendido del Led es: %d <h5>", tiempoLed);
  mg_printf(c, "<p><a href='test'><span>Click aquí</span></a></p>");
//  c->flags |= (MG_F_SEND_AND_CLOSE | MGOS_F_RELOAD_CONFIG);
  (void) user_data;
}


*/
//TODO: tiempo de espera en una variable, poner en otra el tiempo de preaviso.

static void cambio_tiempo_destello(void *arg){
  mgos_clear_timer(id);
  tiempoLed=50;
  timercb=mgos_set_timer(tiempoLed, MGOS_TIMER_REPEAT, encender_led, NULL);

(void) arg;
}



static void inicio_bomba(void *arg)
    {
      // flanco ascendente
      contador=0;
      encender_led(NULL);
      id=mgos_set_timer(tiempoLed, MGOS_TIMER_REPEAT, encender_led, NULL);
      mgos_set_timer(tiempoEspera-tiempoPreaviso, 0, cambio_tiempo_destello, NULL);
      mgos_set_timer(tiempoEspera, 0, fin_secuencia_bomba, NULL);

      mgos_gpio_write(LED_PIN, false);

      (void) arg;
    }

  static void mostrar_pulsaciones(void *arg){
    // aca vamos a hacer un switch para saber cuantoas veces es pulso el boton y en funcion de eso acutar
    char str[20];
    id=0;
    switch (contador) {
        case 1:   sprintf(str, "publicamos los valores sensados");
                  LOG(LL_INFO, (str));
                  mgos_mqtt_pubf("Sensor", 1, false, "{advice: %Q, pulsaciones: %d}", mgos_sys_config_get_device_id(), contador);
                  break;
        case 2:   sprintf(str, "Invertimos el valor del led");
                  LOG(LL_INFO, (str));
                  break;
        case 3:   sprintf(str, "Invertimos el valor del cooler");
                  LOG(LL_INFO, (str));
                  break;
        default:  break;
    }

  //  char msj[50];
  //  json_printf(msj, "{advice: %Q, pulsaciones: %d, "nodemcu1", 5);
  //  mgos_mqtt_pub("Sensor", msj, 40, 1, false);

  mgos_mqtt_pubf("Sensor", 1, false, "{advice: %Q, pulsaciones: %d}", mgos_sys_config_get_device_id(), contador);
  (void) arg;

}

    static void cb_boton_liberado(int pin, void *arg)
    {
      // cuando lo suelto comparo el tiempo que estuvo presionado y actuo en consecuencia
        if((mgos_uptime_micros()/1000)-tiempoInicio>=1000)
        {
          // paso mas de 1 segundo entonces inicion la bomba
          if(secuencia_bomba_en_accion) return;
          secuencia_bomba_en_accion=true;
          char str[20];
          sprintf(str, "bomba iniciada");
          LOG(LL_INFO, (str));
          inicio_bomba(NULL);
          DESACTIVAR_COOLER1;

        }
        else
        {
            // paso menos de un segundo etononces hago lo pertinente a contar la pulsacion
            char str[20];
            sprintf(str, "contar pulsacion");
            LOG(LL_INFO, (str));

            if(!id) contador = 0;

            contador++;
            if(id) mgos_clear_timer(id);
            id=mgos_set_timer(1000, 0, mostrar_pulsaciones, NULL);
        }

      // YA DETECTAMOS LA LECTURA DE LIBERRAR EL BOTON, AHORA VOLVEMOS A DEFINIR EL HANDLER PARA CUANDO SE PRESIONE

      mgos_gpio_set_button_handler(PIN_BOTON, MGOS_GPIO_PULL_UP,
                                    MGOS_GPIO_INT_EDGE_NEG,
                                    100, cb_boton_pulsado,
                                    NULL);

      (void) arg;
      (void) pin;

    }


    static void cb_boton_pulsado(int pin, void *arg){
    tiempoInicio=mgos_uptime_micros()/1000;
      // definimos el hablder para el flanco del boton soltado
    mgos_gpio_set_button_handler(PIN_BOTON, MGOS_GPIO_PULL_UP,
                                  MGOS_GPIO_INT_EDGE_POS,
                                  50, cb_boton_liberado,
                                  NULL);
      if(BOTON_PRESIONADO)
      {
        temp = mgos_dht_get_temp(dht);
        hum = mgos_dht_get_humidity(dht);
        luz = mgos_adc_read(PIN_LDR);
        char str[20];
        sprintf(str, "Temperatura: %2.2f", temp);
        LOG(LL_INFO, (str));
        sprintf(str, "Humedad: %2.2f", hum);
        LOG(LL_INFO, (str));
        sprintf(str, "LUZ: %d", luz);
        LOG(LL_INFO, (str));
        ACTIVAR_COOLER_1;

      } else {
        char str[20];
        sprintf(str, "no pulsado");
        LOG(LL_INFO, (str));
      }


      (void) arg;
      (void) pin;
    }



enum mgos_app_init_result mgos_app_init(void)
    {
#ifdef LED_PIN
    mgos_gpio_setup_output(LED_PIN, 0);
#endif
    CONFIGURAR_COOLER1;
    DESACTIVAR_COOLER1;
    //mgos_set_timer(2000 /* ms */, MGOS_TIMER_REPEAT, encender_led, NULL);
    mgos_gpio_write(LED_PIN, true);


    mgos_gpio_set_button_handler(PIN_BOTON, MGOS_GPIO_PULL_UP,
                                  MGOS_GPIO_INT_EDGE_NEG,
                                  100, cb_boton_pulsado,
                                  NULL);
//    mgos_register_http_endpoint("/foo", foo_handler, NULL);
//    mgos_register_http_endpoint("/test", test, NULL);
    dht=mgos_dht_create(PIN_DHT, DHT11);
    mgos_adc_enable(PIN_LDR);
    return MGOS_APP_INIT_SUCCESS;
    }
