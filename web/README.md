Esta parte del proyecto es el interface web del PortalDoor
Una interface muy sencilla para gestionar la apertura de la puerta del portal desde el teléfono movil (sin llaves)
Básicamente lo que hay es un ESP32, conectado a un rele. Este microcontrolador tiene un sencillo servidor
web con dos peticiones que se le pueden hacer:

http://direccionip/status (devuelve { "result":"ok","resultcode":"0","alive":"1","opening":"0" } )
http://direccionip/open (devuelve { "result":"ok","resultcode":"0","relaytime":"4000" } )

en realidad, por dar un minimo de seguridad, el servidor web esta programado para que se le ponga una password en las url...
aunque no lo uso porque no creo que merezca la pena.

El javascript tiene contemplado el poder hacer la petición tanto por dirección externa, como dirección interna
(es un poco lio, pero al final básicamente lo que se ha hecho es solicitar ambas peticiones a la vez y no controlar errores).
El problema se daba en que desde el portal a veces me coje la wifi de casa.

Un puñetero lio.

Para instalar la aplicación en un iphone ir a la dirección web del servidor (192.168.4.100/portaldoor/) y una vez que
accedas ahí dar al cuadrado con la flecha (debajo justo en el medio) y después a "Adadir a pantalla de inicio"
(hay que hacer scroll para encontrar el icono).

Cosas pendientes:
 - cambiar icono aplicación
 - Mejorar aspecto de la página web (por ahora es sólo una imagen)
 - Gestionar cuando sepamos que estamos en casa o en la calle no solicitar al otro
 - en status mientras este fallando uno no hacerle muchas peticiones
 - organizar el codigo un poco
 

