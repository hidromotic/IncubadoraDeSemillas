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
#include "mgos_pwm.h"
#include "mgos_cron.h"
//////////////////////************* ACA PONEMOS TODOS LOS DEFINE ***************////////////////////

#define PIN_BOTON 0
#define LED_PIN 2
#define ENCENDER_LED_TESTEO mgos_gpio_write(LED_PIN, false)
#define APAGAR_LED_TESTEO mgos_gpio_write(LED_PIN, true)
#define BOTON_PRESIONADO mgos_gpio_read(PIN_BOTON)==0
#define PIN_DHT 5 // es igual a D1
#define DESTELLO 50
#define PIN_LDR 0 // A0 es igual a 17
#define PIN_COOLER 15 // GPIO15 = D8
#define PIN_COOLER_2 12 // GPIO12 = D6
#define PIN_TIRA_LED 4 // GPIO04 = D2
#define CONFIGURAR_PIN_LED mgos_gpio_setup_output(PIN_TIRA_LED, 0)
#define ACTIVAR_COOLER_1 mgos_gpio_write(PIN_COOLER, true)
#define DESACTIVAR_COOLER1 mgos_gpio_write(PIN_COOLER, false)
#define CONFIGURAR_COOLER1 mgos_gpio_setup_output(PIN_COOLER, 0)
#define INVERTIR_LED mgos_gpio_toggle(LED_PIN)

// topicos donde nos subscribimos
#define TOPICO_LUZ "LUZ"
#define TOPICO_COOLER "COOLER"
#define TOPICO_AUTOMATICO_MANUAL "AUTOMATICO"
#define TOPICO_TIEMPO_ENCENDIDO "TIEMPO_ENCENDIDO"
#define TOPICO_TIEMPO_APAGADO "TIEMPO_APAGADO"
#define TOPICO_UMBRAL_LUZ "UMBRAL_LUZ"
#define TOPICO_MARGEN_LUZ "MARGEN_LUZ"
#define TOPICO_UMBRAL_TEMP "UMBRAL_TEMPERATURA"
#define TOPICO_MARGEN_TEMP "MARGEN_TEMPERATURA"

// topicos donde publicamos
#define PUB_TOPIC_LUZ "iluminacion"
#define PUB_TOPIC_TEMP "Temperatura"
#define PUB_TOPIC_HUM "Humedad"
#define PUB_TOPIC_SALIDA_COOLER "SALIDA_COOLER"
#define PUB_TOPIC_SALIDA_LED "SALIDA_LED"

#define ESTADO_CONTROL_MANUAL 1
#define ESTADO_CONTROL_AUTOMATICO 0
#define INTERVALO_DE_PUBLICACION 1000

// Umbrales de luz y Temperatura
int umbral_luz=600;
int margen_luz=100;
int umbral_temp=23;
int margen_temp=7;

//////////////////////************* ACA PONEMOS TODAS LAS VARIABLES GLOBALES ***************////////////////////

bool encendidoFijo=false;
int tiempoLed = 500;
mgos_timer_id id=NULL;
mgos_timer_id soltar=NULL;
mgos_timer_id tmpBomba=NULL;
mgos_timer_id apagarBoton=NULL;
mgos_timer_id timercb=NULL;
mgos_timer_id automatico_encendido=NULL;
bool secuencia_bomba_en_accion=false;




int estado_cooler=0;
int estado_led=0;
bool estado_automatico=ESTADO_CONTROL_MANUAL;

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
static void led_test(void *arg);
static void apagar_led(void *arg);
static void control_luz(void *arg);
static void publicar(void *arg);
static void actualizar_salidas(void *arg);
static void automatico_en_accion(void *arg);
static void leer_sensores(void *arg);
/*
static void mostrar_tiempo(void *arg){
//  time_t tiempo = mg_time();


  char str[80];
  sprintf(str, "tiempo Hora: %d:%d:%d", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
  LOG(LL_INFO, (str));
  sprintf(str, "tiempo Dia: %d", tmp->tm_mday);
  LOG(LL_INFO, (str));
  (void) arg;
}*/

static void actualizar_salidas(void *arg){
  mgos_gpio_write(PIN_COOLER, estado_cooler);

  float duty=estado_led/255.0;
  char str[50];
  sprintf(str, "Estado de LED: %d - %f", estado_led, duty);
//  LOG(LL_INFO, (str));

  mgos_pwm_set(PIN_TIRA_LED, 1000, duty);

  (void) arg;
}

static void automatico_en_accion(void *arg){
  control_luz(NULL);
  // control de la temperatura
  if(temp>(umbral_temp+margen_temp)){
    estado_cooler=1;
  } else {
    estado_cooler=0;
  }

  (void) arg;
}

static void leer_sensores(void *arg){
  static int contador=0;
  static int luz_parcial=0;
  luz_parcial+= mgos_adc_read(PIN_LDR);
  contador++;
  if(contador==10){
    contador=0;
    luz = luz_parcial/10;
    luz_parcial=0;
  }
  temp = mgos_dht_get_temp(dht);
  hum = mgos_dht_get_humidity(dht);
  (void) arg;
}

static void publicar(void *arg){
  char str[20];
  sprintf(str, "Temperatura: %2.2f", temp);
  LOG(LL_INFO, (str));
  sprintf(str, "Humedad: %2.2f", hum);
  LOG(LL_INFO, (str));
  sprintf(str, "LUZ: %d", luz);
  LOG(LL_INFO, (str));

  mgos_mqtt_pubf(PUB_TOPIC_TEMP, 1, false, "%f", temp);
  mgos_mqtt_pubf(PUB_TOPIC_HUM, 1, false, "%f", hum);
  mgos_mqtt_pubf(PUB_TOPIC_LUZ, 1, false, "%d", luz);

  mgos_mqtt_pubf("Sensor", 1, false, "{advice: %Q, luz: %d, humedad: %f, temperatura: %f}", mgos_sys_config_get_device_id(), luz, hum, temp);
  (void) arg;

}

static void control_luz(void *arg){
  if(luz<(umbral_luz-margen_luz)){
    if(estado_led>5) estado_led-=5;
  }
  if(luz>(umbral_luz+margen_luz)){
    if(estado_led<250) estado_led+=5;
  }
  (void) arg;
}

static void led_test(void *arg){
  ENCENDER_LED_TESTEO;
  mgos_set_timer(50, 0, apagar_led, NULL);
  (void) arg;
}

static void apagar_led(void *arg){
  APAGAR_LED_TESTEO;
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

    mgos_gpio_write(LED_PIN, false);  //REEMPLAZAR POR --> ENCENDER_LED_TEST o APAGAR_LED_TEST , según corresponda
    apagarBoton=mgos_set_timer(DESTELLO, 0, apagar_led, NULL);

    (void) arg;
    }
/*
static void accion_ajustes(struct mg_connection *nc, const char *topic,
                                                        int topic_len, const char *msg, int msg_len,
                                                        void *ud)
    {
    char str[50];

    sprintf(str, "recibido de %.*s -->%.*s", topic_len, topic, msg_len, msg);
    LOG(LL_INFO, (str));
    (void) ud;
    (void) nc;
    (void) topic;
    (void) topic_len;
    (void) msg;
    (void) msg_len;

}*/
static void ejemplo_cron(void *user_data, mgos_cron_id_t id){
  time_t epoch = (time_t) mg_time();
  struct tm *tmp = localtime(&epoch);

  char str[80];
  sprintf(str, "tiempo Hora: %d:%d:%d", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
  LOG(LL_INFO, (str));
  sprintf(str, "tiempo Dia: %d", tmp->tm_mday);
  LOG(LL_INFO, (str));
  (void) id;
  (void) user_data;
}

static void ajustes_margen_temp(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){


  int nuevo_margen= atoi(msg);
  margen_temp=nuevo_margen;

(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}

static void ajustes_umbral_temp(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){


  int nuevo_umbral= atoi(msg);
  umbral_temp=nuevo_umbral;

(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}

static void ajustes_umbral_luz(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){


  int nuevo_umbral= atoi(msg);
  umbral_luz=nuevo_umbral;

(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}

static void ajustes_margen_luz(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){


int nuevo_margen= atoi(msg);
margen_luz=nuevo_margen;
(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}

static void ajustes_cooler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){
char str[50];



int nuevo_estado= atoi(msg);
if(estado_automatico==ESTADO_CONTROL_AUTOMATICO) return;
if(nuevo_estado==0 || nuevo_estado==1){
  estado_cooler=nuevo_estado;
}

sprintf(str, "recibido de %.*s -->%.*s", topic_len, topic, msg_len, msg);
LOG(LL_INFO, (str));



(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}

static void ajustes_led(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){
char str[50];

int nuevo_estado= atoi(msg);

  if(estado_automatico==ESTADO_CONTROL_AUTOMATICO) return;
  estado_led=nuevo_estado;




sprintf(str, "recibido de %.*s -->%.*s", topic_len, topic, msg_len, msg);
LOG(LL_INFO, (str));

(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}

static void ajustes_automatico(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud){
char str[50];

int nuevo_estado= atoi(msg);

if(nuevo_estado==ESTADO_CONTROL_MANUAL){
  estado_automatico=ESTADO_CONTROL_MANUAL;
  mgos_clear_timer(automatico_encendido);
} else if(nuevo_estado==ESTADO_CONTROL_AUTOMATICO) {
  estado_automatico=ESTADO_CONTROL_AUTOMATICO;
  automatico_encendido=mgos_set_timer(1000, MGOS_TIMER_REPEAT, automatico_en_accion, NULL);
}

sprintf(str, "recibido de %.*s -->%.*s", topic_len, topic, msg_len, msg);
LOG(LL_INFO, (str));

(void) ud;
(void) nc;
(void) topic;
(void) topic_len;
(void) msg;
(void) msg_len;

}


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
                  mgos_mqtt_pubf("Sensor", 1, false, "{advice: %Q, luz: %d, humedad: %f, temperatura: %f}", mgos_sys_config_get_device_id(), luz, hum, temp);
                  break;
        case 2:   sprintf(str, "Invertimos el valor del led");
                  LOG(LL_INFO, (str));
                  INVERTIR_LED;
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
        temp = 2.5;//mgos_dht_get_temp(dht);
        hum = 1.20;//mgos_dht_get_humidity(dht);
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
    CONFIGURAR_PIN_LED;
    DESACTIVAR_COOLER1;
    setenv("TZ", "America/Argentina/Buenos_Aires", 1);
    tzset();
    //mgos_set_timer(2000 /* ms */, MGOS_TIMER_REPEAT, encender_led, NULL);
    mgos_gpio_write(LED_PIN, true);

    mgos_set_timer(2000, MGOS_TIMER_REPEAT, led_test, NULL);

    mgos_set_timer(100, MGOS_TIMER_REPEAT, actualizar_salidas, NULL);

    mgos_set_timer(INTERVALO_DE_PUBLICACION, MGOS_TIMER_REPEAT, publicar, NULL);

    mgos_set_timer(10, MGOS_TIMER_REPEAT, leer_sensores, NULL);

//    mgos_set_timer(1000, MGOS_TIMER_REPEAT, mostrar_tiempo, NULL);

    mgos_gpio_set_button_handler(PIN_BOTON, MGOS_GPIO_PULL_UP,
                                  MGOS_GPIO_INT_EDGE_NEG,
                                  100, cb_boton_pulsado,
                                  NULL);


    // mgos_mqtt_sub("config", accion_ajustes, NULL);
    mgos_mqtt_sub(TOPICO_COOLER, ajustes_cooler, NULL);
    mgos_mqtt_sub(TOPICO_LUZ, ajustes_led, NULL);
    mgos_mqtt_sub(TOPICO_AUTOMATICO_MANUAL, ajustes_automatico, NULL);
    mgos_mqtt_sub(TOPICO_UMBRAL_LUZ, ajustes_umbral_luz, NULL);
    mgos_mqtt_sub(TOPICO_MARGEN_LUZ, ajustes_margen_luz, NULL);
    mgos_mqtt_sub(TOPICO_UMBRAL_TEMP, ajustes_umbral_temp, NULL);
    mgos_mqtt_sub(TOPICO_MARGEN_TEMP, ajustes_margen_temp, NULL);
    mgos_cron_add("*/5 5-59 22-23 * * THU-SAT", ejemplo_cron, NULL);

//    mgos_register_http_endpoint("/foo", foo_handler, NULL);
//    mgos_register_http_endpoint("/test", test, NULL);
    dht=mgos_dht_create(PIN_DHT, DHT11);
    mgos_adc_enable(PIN_LDR);
    return MGOS_APP_INIT_SUCCESS;
    }
