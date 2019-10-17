# IncubadoraDeSemillas
Control Automático para Incubadora de Semillas (ITES 2019)  
Deberá estar funcionando para la feria del 30 de Octubre del 2019  

# REQUERIMIENTOS MÍNIMOS

# Funcionalidad
### Led Test
Se contará con un led que destellará cada 2 segundos. Se mantendrá encendido durante 50ms, para indicar que el sistema está funcionando de manera apropiada.  

### Control de Humedad / Temperatura
Si el control automático está habilitado, activará o desactivará los coolers en función del valor de ...  
Se podrá comandar manualmente la activación de los coolers  

### Control de Luz
Para controlar el modo automático de la luz se usaran como referencia 2 constantes. 
La primera la llamaremos L y será un valor medio, y la segunda la llamaremos M y será un valor de margen a considerar. 
Si se habilita el control automático, la lampara debe encender cuando la luz ambiente se encuentre por debajo de (L - M) y se debe apagar cuando la luz ambiente esté por encima de (L + M)
En todo caso se podrá comandar manualmente el encendido y apagado de la lámpara.
Se muestreará el nivel de luz con un período de 1 segundo


# Visualización WEB
Temperatura de... en tal unidad  
Humedad de... en tal humedad  
Instante del último encendido de....  


# Ajustes del sistema (por módulo)
## Control de Temperatura
Especificación de valores de temperatura  

## Control de Humedad
Especificación de valores de humedad  
Habilitar/Deshabilitar el control automático de...  
Valor de alarma para...  
Fecha y Hora...  
Horarios de encendido...  
Duracion de   

# Sensores/Entradas
- DHT11 --> Sensor de Humedad y Temperatura [I2C--> SCK=GPIOx, SDA= GPIOx]
- LDR --> Sensor de Luz [entrada ADC, GPIOx del ESP]
- Pulsador [GPIOx del ESP]

# Actuadores
- Lámpara de Led [GPIOx]
- Coolers (entrada/salida) [GPIOx]
- Led Test [GPIOx]

# Comunicación
Se comunicará con un servidor con Mosquito, para publicar mensajes MQTT.  
Los tópicos que utilizará serán:  
- /temperatura  
- /humedad  
- /alarmas  
El mensaje será en formato JSON clave/valor. Por ejemplo:
{ "humedad": "ff.ff",
  "temperatura": "ff.ff"
  "luz": "d" }
Donde 'f' se refiere a un numero de punto flotante y 'd' a un entero 
El mensaje se enviará cuando...  
El sistema se suscribirá a los tópicos para recibir comandos y ajustes desde la web:
  - /config 
  - suscripcion1
  - suscripcion2
  - ...

Se deberán procesar los siguientes mensajes
"ctrl_humedad":false,   //Para habilitar/deshabilitar el control automático de Humedad
"ctrl_luz":true,        //Para habilitar/deshabilitar el control automático de Luz

Los datos de conexión al servidor son:  

# Hardware
Se alimentará desde un transformador de 12V.  
Se utilizará un módulo Step Down para bajar a 6V, que alimentará la lámpara de Ledy los Coolers.  
El NodeMCU se alimentará desde ....

Batería?? Voltaje??? Autonomía esperada?? Función de la batería???  

# Modo Desarrollo
- Un toque de botón --> Publicar el valor de Temperatura+Humedad+LDR
- Dos toques de botón --> Invertir el estado de la lámpara de Led
- Tres toques de botón --> Invertir el estado de los Coolers
