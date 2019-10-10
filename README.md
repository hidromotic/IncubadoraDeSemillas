# IncubadoraDeSemillas
Control Automático para Incubadora de Semillas (ITES 2019)  
Deberá estar funcionando para la feria del 30 de Octubre del 2019  

# REQUERIMIENTOS MÍNIMOS

# Funcionalidad
### Led Test
Se contará con un led que destellará cada 2 segundos. Se mantendrá encendido durante 50ms, para indicar que el sistema está funcionando de manera apropiada.  

### Control de Humedad / Temperatura
Si el control automático de.... está habilitado, activará o desactivará los coolers en función del valor de ...  
Se podrá comandar manualmente la activación de los coolers  

### Control de Luz
Se podrá comandar manualmente la activación de la lámpara


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
El formato de los mensajes será....  
El mensaje se enviará cuando...  
El sistema se suscribirá a los tópicos para recibir comandos y ajustes desde la web:
- suscripcion1
- suscripcion2
- ...
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
