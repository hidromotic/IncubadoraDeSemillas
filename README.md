# IncubadoraDeSemillas
Control Automático para Incubadora de Semillas (ITES 2019)  
Deberá estar funcionando para la feria del 30 de Octubre del 2019  

# Funcionalidad
### Led Test
Se contará con un led que destellará cada 2 segundos. Se mantendrá encendido durante 50ms, para indicar que el sistema está funcionando de manera apropiada.  

### Control de Humedad
Si el control automático de.... está habilitado, activará o desactivará ...  

### Control de Luz
Se podrá activar manualmente...  


# Visualización WEB
Temperatura de... en tal unidad  
Humedad de... en tal humedad  
Instante del último encendido de....  


# Ajustes del sistema (por módulo)
## Control de Temperatura
Especificación de valo-res de temperatura  
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
Los datos de conexión al servidor son:  

# Hardware
Se alimentará desde un transformador de 12V.  
Se utilizará un módulo Step Down para bajar a 6V, que alimentará la lámpara de Ledy los Coolers.  
El NodeMCU se alimentará desde ....

Batería?? Voltaje??? Autonomía esperada?? Función de la batería???  
