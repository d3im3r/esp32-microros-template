#include <Arduino.h>
#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/int32.h>
#include "MicroROSHelper.h"

// 1. CONFIGURACIÓN: adapte la red y la IP del computador que ejecuta el Agent.
// El dominio debe coincidir con ROS_DOMAIN_ID en las terminales ROS 2.
const size_t ROS_DOMAIN = 13;
const size_t EXECUTOR_HANDLES = 2; // 1 subscriber + 1 timer.
char WIFI_SSID[] = "NOMBRE_RED";
char WIFI_PASSWORD[] = "CLAVE_RED";
IPAddress AGENT_IP(10,41,3,65);
const uint16_t AGENT_PORT = 8888;
const char NODE_NAME[] = "mi_nodo";

const uint8_t LED_PIN = 2;
const uint32_t COUNT_PERIOD_MS = 1000; // Período en milisegundos: 1000 ms = 1 s.

// 2. MENSAJES Y ENTIDADES: Int32 para el conteo y Bool para la orden del LED.
MicroROSHelper ros;
std_msgs__msg__Int32 contador_msg;
std_msgs__msg__Bool led_msg;
rcl_publisher_t contador_pub;
rcl_subscription_t led_sub;
rcl_timer_t contador_timer;
int32_t contador = 0;

// Subscriber: true enciende el LED; false lo apaga.
void led_callback(const void * msgin)
{
    if (msgin == nullptr) return;
    // El callback recibe un puntero genérico. Lo interpretamos como mensaje Bool,
    // el mismo tipo registrado al crear el subscriber.
    const std_msgs__msg__Bool * msg = (const std_msgs__msg__Bool *) msgin;

    // data contiene la orden recibida: true = encender, false = apagar.
    if (msg->data) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}

// Publisher: envía el conteo una vez por segundo, independientemente del LED.
void contador_callback(rcl_timer_t * timer, int64_t last_call_time)
{
    (void) last_call_time;
    if (timer == nullptr) return;

    // Reiniciar al alcanzar el límite evita desbordar un entero con signo.
    contador = (contador == INT32_MAX) ? 0 : contador + 1;
    // Copiar el valor al campo data antes de publicar el mensaje.
    contador_msg.data = contador;
    if (!ros.publish(contador_pub, &contador_msg)) {
        ros.info("Error publicando /contador");
    }
}

void setup()
{
    // 3. HARDWARE: configurar GPIO 2 como salida y comenzar con el LED apagado.
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // 4. CONEXIÓN: inicializar micro-ROS. require() detiene el programa si falla.
    bool ok = ros.beginWiFi(
        NODE_NAME, EXECUTOR_HANDLES, ROS_DOMAIN,
        WIFI_SSID, WIFI_PASSWORD, AGENT_IP, AGENT_PORT
    );
    ros.require(ok, "Error inicializando micro-ROS");

    // 5. PUBLISHER: publica mensajes Int32 en /contador.
    ok = ros.createPublisher(
        contador_pub,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "/contador"
    );
    ros.require(ok, "Error creando publisher /contador");

    // 6. SUBSCRIBER: guarda el Bool en led_msg y ejecuta led_callback al recibirlo.
    ok = ros.createSubscription(
        led_sub,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "/led", &led_msg, led_callback
    );
    ros.require(ok, "Error creando subscriber /led");

    // 7. TIMER: llama periódicamente a contador_callback para publicar el conteo.
    ok = ros.createTimer(contador_timer, COUNT_PERIOD_MS, contador_callback);
    ros.require(ok, "Error creando timer de conteo");
    ros.info("Nodo listo");
}

void loop()
{
    if (!ros.wifiConnected()) {
        ros.info("Se perdio la conexion WiFi");
        delay(1000);
        return;
    }
    // 8. EXECUTOR: atiende el subscriber y el timer. Evite tareas largas aquí
    // y dentro de los callbacks para que ambos puedan ejecutarse a tiempo.
    ros.spinSome(10);
    delay(10);
}
