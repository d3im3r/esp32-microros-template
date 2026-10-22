#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rcl/init_options.h>
#include <rcl/publisher.h>
#include <rcl/subscription.h>
#include <rcl/timer.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rosidl_runtime_c/message_type_support_struct.h>


/*
 * ============================================================
 * MicroROSHelper.h
 * ============================================================
 *
 * Helper didáctico para micro-ROS + ESP32 + PlatformIO.
 *
 * Encapsula:
 *
 *   - transporte Serial o WiFi;
 *   - ROS_DOMAIN_ID;
 *   - allocator;
 *   - init options;
 *   - support;
 *   - node;
 *   - executor;
 *   - creación de publishers;
 *   - creación de subscriptions;
 *   - creación de timers;
 *   - publicación;
 *   - spin;
 *   - diagnóstico básico.
 *
 * El estudiante continúa definiendo:
 *
 *   - tipo de mensaje;
 *   - nombre de tópicos;
 *   - mensajes;
 *   - publishers;
 *   - subscribers;
 *   - timers;
 *   - callbacks;
 *   - lógica de aplicación.
 *
 * ============================================================
 */


class MicroROSHelper
{
public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    MicroROSHelper()
    {
        initialized_ = false;
        executor_ready_ = false;

        executor_handles_ = 0;
        domain_id_ = 0;

        last_error_ = RCL_RET_OK;
        last_step_ = "none";

        node_ =
            rcl_get_zero_initialized_node();

        executor_ =
            rclc_executor_get_zero_initialized_executor();

        init_options_ =
            rcl_get_zero_initialized_init_options();
    }


    // ========================================================
    // INICIALIZACIÓN POR SERIAL
    // ========================================================

    bool beginSerial(
        const char * node_name,
        size_t executor_handles,
        size_t domain_id,
        HardwareSerial & serial = Serial,
        uint32_t baudrate = 115200,
        const char * node_namespace = "")
    {

#if defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)

        domain_id_ =
            domain_id;

        executor_handles_ =
            executor_handles;


        serial.begin(
            baudrate
        );


        delay(
            2000
        );


        // ----------------------------------------------------
        // Configurar transporte micro-ROS
        // ----------------------------------------------------

        set_microros_serial_transports(
            serial
        );


        // ----------------------------------------------------
        // Inicializar infraestructura común
        // ----------------------------------------------------

        return initCore(
            node_name,
            node_namespace,
            executor_handles,
            domain_id
        );


#else

        (void) node_name;
        (void) executor_handles;
        (void) domain_id;
        (void) serial;
        (void) baudrate;
        (void) node_namespace;


        return localError(
            "Serial transport not enabled"
        );

#endif
    }


    // ========================================================
    // INICIALIZACIÓN POR WIFI
    // ========================================================

    bool beginWiFi(
        const char * node_name,
        size_t executor_handles,
        size_t domain_id,
        char * ssid,
        char * password,
        IPAddress agent_ip,
        uint16_t agent_port = 8888,
        const char * node_namespace = "")
    {

#if defined(MICRO_ROS_TRANSPORT_ARDUINO_WIFI)

        // ----------------------------------------------------
        // Guardar configuración desde el inicio
        // ----------------------------------------------------

        domain_id_ =
            domain_id;

        executor_handles_ =
            executor_handles;


        // ====================================================
        // 1. SERIAL PARA DEPURACIÓN
        // ====================================================

        Serial.begin(
            115200
        );


        delay(
            500
        );


        Serial.println();

        Serial.println(
            "========================================"
        );

        Serial.println(
            "        INICIO micro-ROS WiFi"
        );

        Serial.println(
            "========================================"
        );


        Serial.print(
            "Nodo          : "
        );

        Serial.println(
            node_name
        );


        Serial.print(
            "ROS_DOMAIN_ID : "
        );

        Serial.println(
            domain_id_
        );


        Serial.print(
            "SSID          : "
        );

        Serial.println(
            ssid
        );


        Serial.print(
            "Agent IP      : "
        );

        Serial.println(
            agent_ip
        );


        Serial.print(
            "Agent puerto  : "
        );

        Serial.println(
            agent_port
        );


        Serial.println(
            "----------------------------------------"
        );


        Serial.println(
            "Conectando a WiFi..."
        );


        // ====================================================
        // 2. CONFIGURAR TRANSPORTE WIFI micro-ROS
        // ====================================================
        //
        // IMPORTANTE:
        //
        // No llamamos WiFi.begin() manualmente.
        //
        // set_microros_wifi_transports() administra la
        // conexión WiFi internamente.
        //
        // Esto evita inicializar la conexión dos veces.
        //
        // ====================================================

        set_microros_wifi_transports(
            ssid,
            password,
            agent_ip,
            agent_port
        );


        // ====================================================
        // 3. WIFI CONECTADO
        // ====================================================
        //
        // Si la función anterior retorna, el transporte
        // WiFi ya fue configurado.
        //
        // ====================================================

        Serial.println(
            "WiFi conectado correctamente."
        );


        Serial.print(
            "IP local      : "
        );

        Serial.println(
            WiFi.localIP()
        );


        Serial.print(
            "Gateway       : "
        );

        Serial.println(
            WiFi.gatewayIP()
        );


        Serial.print(
            "RSSI          : "
        );

        Serial.print(
            WiFi.RSSI()
        );

        Serial.println(
            " dBm"
        );


        // ====================================================
        // 4. INICIALIZAR micro-ROS
        // ====================================================

        Serial.println(
            "----------------------------------------"
        );


        Serial.println(
            "Inicializando infraestructura micro-ROS..."
        );


        bool ok =
            initCore(
                node_name,
                node_namespace,
                executor_handles,
                domain_id
            );


        if (!ok)
        {
            return false;
        }


        // ====================================================
        // 5. RESULTADO
        // ====================================================

        Serial.println(
            "----------------------------------------"
        );


        Serial.println(
            "micro-ROS inicializado correctamente."
        );


        Serial.print(
            "Nodo          : "
        );

        Serial.println(
            node_name
        );


        Serial.print(
            "ROS_DOMAIN_ID : "
        );

        Serial.println(
            domain_id_
        );


        Serial.println(
            "========================================"
        );


        return true;


#else

        (void) node_name;
        (void) executor_handles;
        (void) domain_id;
        (void) ssid;
        (void) password;
        (void) agent_ip;
        (void) agent_port;
        (void) node_namespace;


        return localError(
            "WiFi transport not enabled"
        );

#endif
    }


    // ========================================================
    // CREAR PUBLISHER
    // ========================================================

    bool createPublisher(
        rcl_publisher_t & publisher,
        const rosidl_message_type_support_t * type_support,
        const char * topic_name)
    {
        if (!initialized_)
        {
            return localError(
                "createPublisher: node not initialized"
            );
        }


        publisher =
            rcl_get_zero_initialized_publisher();


        rcl_ret_t rc =
            rclc_publisher_init_default(
                &publisher,
                &node_,
                type_support,
                topic_name
            );


        return check(
            rc,
            "createPublisher"
        );
    }


    // ========================================================
    // PUBLICAR MENSAJE
    // ========================================================

    bool publish(
        rcl_publisher_t & publisher,
        const void * message)
    {
        rcl_ret_t rc =
            rcl_publish(
                &publisher,
                message,
                nullptr
            );


        return check(
            rc,
            "publish"
        );
    }


    // ========================================================
    // CREAR SUBSCRIPTION
    // ========================================================

    bool createSubscription(
        rcl_subscription_t & subscription,
        const rosidl_message_type_support_t * type_support,
        const char * topic_name,
        void * receive_message,
        rclc_subscription_callback_t callback,
        rclc_executor_handle_invocation_t invocation =
            ON_NEW_DATA)
    {
        if (!initialized_)
        {
            return localError(
                "createSubscription: node not initialized"
            );
        }


        if (!executor_ready_)
        {
            return localError(
                "createSubscription: executor not initialized"
            );
        }


        subscription =
            rcl_get_zero_initialized_subscription();


        // ----------------------------------------------------
        // Crear subscription
        // ----------------------------------------------------

        rcl_ret_t rc =
            rclc_subscription_init_default(
                &subscription,
                &node_,
                type_support,
                topic_name
            );


        if (!check(
                rc,
                "createSubscription:init"))
        {
            return false;
        }


        // ----------------------------------------------------
        // Registrar subscription en executor
        // ----------------------------------------------------

        rc =
            rclc_executor_add_subscription(
                &executor_,
                &subscription,
                receive_message,
                callback,
                invocation
            );


        return check(
            rc,
            "createSubscription:addToExecutor"
        );
    }


    // ========================================================
    // CREAR TIMER
    // ========================================================

    bool createTimer(
        rcl_timer_t & timer,
        uint32_t period_ms,
        rcl_timer_callback_t callback)
    {
        if (!initialized_)
        {
            return localError(
                "createTimer: node not initialized"
            );
        }


        if (!executor_ready_)
        {
            return localError(
                "createTimer: executor not initialized"
            );
        }


        timer =
            rcl_get_zero_initialized_timer();


        // ----------------------------------------------------
        // Crear timer
        // ----------------------------------------------------

        rcl_ret_t rc =
            rclc_timer_init_default(
                &timer,
                &support_,
                RCL_MS_TO_NS(
                    period_ms
                ),
                callback
            );


        if (!check(
                rc,
                "createTimer:init"))
        {
            return false;
        }


        // ----------------------------------------------------
        // Registrar timer en executor
        // ----------------------------------------------------

        rc =
            rclc_executor_add_timer(
                &executor_,
                &timer
            );


        return check(
            rc,
            "createTimer:addToExecutor"
        );
    }


    // ========================================================
    // SPIN DEL EXECUTOR
    // ========================================================

    bool spinSome(
        uint32_t timeout_ms = 10)
    {
        if (!executor_ready_)
        {
            return true;
        }


        rcl_ret_t rc =
            rclc_executor_spin_some(
                &executor_,
                RCL_MS_TO_NS(
                    timeout_ms
                )
            );


        return check(
            rc,
            "spinSome"
        );
    }


    // ========================================================
    // REQUIRE
    // ========================================================

    void require(
        bool condition,
        const char * message)
    {
        if (condition)
        {
            return;
        }


#if defined(MICRO_ROS_TRANSPORT_ARDUINO_WIFI)

        Serial.println();

        Serial.println(
            "========================================"
        );

        Serial.println(
            "           ERROR micro-ROS"
        );

        Serial.println(
            "========================================"
        );


        Serial.print(
            "Mensaje : "
        );

        Serial.println(
            message
        );


        Serial.print(
            "Paso    : "
        );

        Serial.println(
            last_step_
        );


        Serial.print(
            "Codigo  : "
        );

        Serial.println(
            (int) last_error_
        );


        Serial.print(
            "Dominio : "
        );

        Serial.println(
            domain_id_
        );


        Serial.println(
            "----------------------------------------"
        );


        Serial.println(
            "Programa detenido."
        );


        Serial.println(
            "Corrija el problema y reinicie."
        );


        Serial.println(
            "========================================"
        );

#endif


        while (true)
        {
            delay(
                1000
            );
        }
    }


    // ========================================================
    // MENSAJE INFORMATIVO
    // ========================================================

    void info(
        const char * message)
    {

#if defined(MICRO_ROS_TRANSPORT_ARDUINO_WIFI)

        Serial.print(
            "[micro-ROS] "
        );

        Serial.println(
            message
        );

#else

        (void) message;

#endif
    }


    // ========================================================
    // ESTADO WIFI
    // ========================================================

    bool wifiConnected() const
    {

#if defined(MICRO_ROS_TRANSPORT_ARDUINO_WIFI)

        return
            WiFi.status() == WL_CONNECTED;

#else

        return false;

#endif
    }


    // ========================================================
    // ACCESO A ENTIDADES
    // ========================================================

    rcl_node_t * node()
    {
        return
            &node_;
    }


    rclc_support_t * support()
    {
        return
            &support_;
    }


    rclc_executor_t * executor()
    {
        return
            &executor_;
    }


    rcl_allocator_t * allocator()
    {
        return
            &allocator_;
    }


    // ========================================================
    // ESTADO
    // ========================================================

    bool isInitialized() const
    {
        return
            initialized_;
    }


    bool hasExecutor() const
    {
        return
            executor_ready_;
    }


    size_t executorHandles() const
    {
        return
            executor_handles_;
    }


    size_t domainID() const
    {
        return
            domain_id_;
    }


    // ========================================================
    // DIAGNÓSTICO
    // ========================================================

    rcl_ret_t lastError() const
    {
        return
            last_error_;
    }


    const char * lastStep() const
    {
        return
            last_step_;
    }



private:


    // ========================================================
    // ENTIDADES FUNDAMENTALES
    // ========================================================

    rcl_allocator_t allocator_;

    rcl_init_options_t init_options_;

    rclc_support_t support_;

    rcl_node_t node_;

    rclc_executor_t executor_;


    // ========================================================
    // ESTADO
    // ========================================================

    bool initialized_;

    bool executor_ready_;

    size_t executor_handles_;

    size_t domain_id_;


    // ========================================================
    // DIAGNÓSTICO
    // ========================================================

    rcl_ret_t last_error_;

    const char * last_step_;


    // ========================================================
    // INICIALIZACIÓN COMÚN
    // ========================================================

    bool initCore(
        const char * node_name,
        const char * node_namespace,
        size_t executor_handles,
        size_t domain_id)
    {

        initialized_ =
            false;


        executor_ready_ =
            false;


        executor_handles_ =
            executor_handles;


        domain_id_ =
            domain_id;


        // ====================================================
        // 1. ALLOCATOR
        // ====================================================

        allocator_ =
            rcl_get_default_allocator();


        // ====================================================
        // 2. INIT OPTIONS
        // ====================================================

        init_options_ =
            rcl_get_zero_initialized_init_options();


        rcl_ret_t rc =
            rcl_init_options_init(
                &init_options_,
                allocator_
            );


        if (!check(
                rc,
                "rcl_init_options_init"))
        {
            return false;
        }


        // ====================================================
        // 3. ROS_DOMAIN_ID
        // ====================================================

        rc =
            rcl_init_options_set_domain_id(
                &init_options_,
                domain_id_
            );


        if (!check(
                rc,
                "rcl_init_options_set_domain_id"))
        {
            return false;
        }


        // ====================================================
        // 4. SUPPORT
        // ====================================================

        rc =
            rclc_support_init_with_options(
                &support_,
                0,
                nullptr,
                &init_options_,
                &allocator_
            );


        if (!check(
                rc,
                "rclc_support_init_with_options"))
        {
            return false;
        }


        // ====================================================
        // 5. NODE
        // ====================================================

        node_ =
            rcl_get_zero_initialized_node();


        rc =
            rclc_node_init_default(
                &node_,
                node_name,
                node_namespace,
                &support_
            );


        if (!check(
                rc,
                "rclc_node_init_default"))
        {
            return false;
        }


        initialized_ =
            true;


        // ====================================================
        // 6. EXECUTOR
        // ====================================================

        if (executor_handles_ > 0)
        {
            executor_ =
                rclc_executor_get_zero_initialized_executor();


            rc =
                rclc_executor_init(
                    &executor_,
                    &support_.context,
                    executor_handles_,
                    &allocator_
                );


            if (!check(
                    rc,
                    "rclc_executor_init"))
            {
                initialized_ =
                    false;


                return false;
            }


            executor_ready_ =
                true;
        }


        return true;
    }


    // ========================================================
    // VERIFICAR RESULTADO
    // ========================================================

    bool check(
        rcl_ret_t rc,
        const char * step)
    {
        last_error_ =
            rc;


        last_step_ =
            step;


        return
            rc == RCL_RET_OK;
    }


    // ========================================================
    // ERROR LOCAL
    // ========================================================

    bool localError(
        const char * step)
    {
        last_error_ =
            RCL_RET_ERROR;


        last_step_ =
            step;


        return false;
    }
};