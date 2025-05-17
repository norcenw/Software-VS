import os, sys, io
import M5
from M5 import lcd
from machine import Pin, I2C
import time
import urequests
import network
import json

# Definizione delle credenziali WiFi
#define SECRET_SSID "FASTWEB-XP3E4k"
#define SECRET_PASS "7tjM4BU5JR"
SSID = "FASTWEB-XP3E4k"
PASSWORD = "7tjM4BU5JR"
ip = "192.168.1.184"

# Inizializzazione del display M5Stack Core2
M5.begin()
lcd.setBrightness(100)
lcd.clear()
lcd.print("Inizializzazione...", 0, 0)

# Configurazione dei pin per i motori stepper
MOTOR1_DIR_PIN = 19
MOTOR1_STEP_PIN = 18
MOTOR2_DIR_PIN = 22
MOTOR2_STEP_PIN = 21

# Impostazione dei pin dei motori
motor1_dir = Pin(MOTOR1_DIR_PIN, Pin.OUT)
motor1_step = Pin(MOTOR1_STEP_PIN, Pin.OUT)
motor2_dir = Pin(MOTOR2_DIR_PIN, Pin.OUT)
motor2_step = Pin(MOTOR2_STEP_PIN, Pin.OUT)
x_speed = 100
x_acc = 1
x_delay = 0.001

primo_counter = 0
first_time = True

# Connessione al WiFi
station = network.WLAN(network.STA_IF)
station.active(True)
lcd.print("Connessione al WiFi...", 0, 20)
while not station.isconnected():
    station.connect(SSID, PASSWORD)
    time.sleep(1)
    lcd.print("Tentativo di connessione...", 0, 40)

lcd.clear()
lcd.print("Connesso a WiFi: {}".format(SSID), 0, 20)
print("Connesso a WiFi: {}".format(SSID))

# Configurazione dell'interfaccia I2C per il TCA9554
i2c = I2C(1, scl=Pin(22), sda=Pin(21), freq=100000)
TCA9554_ADDR = 0x20  # Indirizzo I2C del TCA9554

# Funzione per scrivere sul TCA9554
def write_tca9554(value):
    i2c.writeto(TCA9554_ADDR, bytes([value]))

# Funzione per leggere dal TCA9554
def read_tca9554():
    return int.from_bytes(i2c.readfrom(TCA9554_ADDR, 1), 'big')

# Configurazione del TCA9554: 4 ingressi (finecorsa), 4 uscite (EN e suddivisione)
# 0b11110000: primi 4 bit come ingressi, ultimi 4 come uscite
write_tca9554(0xF0)

# Funzione per controllare un motore stepper
def step_motor(step_pin, dir_pin, steps, direction, delay):
    """
    step_pin: pin del segnale di step
    dir_pin: pin del segnale di direzione
    steps: numero di passi da effettuare
    direction: direzione del movimento (0 o 1)
    delay: ritardo tra un passo e l'altro (in secondi)
    """
    dir_pin.value(direction)
    for _ in range(steps):
        step_pin.value(1)
        time.sleep(delay)
        step_pin.value(0)
        time.sleep(delay)

def stop_motors():
    """
    Disabilita i motori stepper impostando i pin di step e direzione a basso.
    """
    motor1_step.value(0)
    motor1_dir.value(0)
    motor2_step.value(0)
    motor2_dir.value(0)
    print("Motori fermati.")


# Funzione per verificare lo stato dei finecorsa
def check_limit_switches():
    """
    Restituisce True se uno qualsiasi dei finecorsa è premuto.
    """
    stop_motors()
    status = read_tca9554()
    # I primi 4 bit rappresentano gli ingressi dei finecorsa
    return (status & 0xF0) != 0xF0

# Funzione per inviare richiesta HTTP finché la risposta non è "1"
def send_http_request_until_success():
    while True:
        try:
            response = urequests.get("http://" + ip + "/op=start_x")
            response_json = response.json()
            response.close()
            
            if response_json.get("status") == 1:
                x_speed = response_json.get("xspeed", 100)  # Velocità del motore stepper (default 100)
                x_acc = response_json.get("xacc", 1)  # Microstep (default 1)
                x_delay = response_json.get("xdelay", 0.001)  # Delay (default 0.001)
                print(f"Risposta HTTP ricevuta. Velocità: {x_speed}, Microstep: {x_acc}, Delay: {x_delay}")
                configure_motor_parameters(x_speed, x_acc, x_delay)
                return x_speed, x_acc, x_delay
            else:
                print("Risposta HTTP non valida: ", response_json)
        except Exception as e:
            print("Errore invio richiesta HTTP: ", e)
            time.sleep(1)

# Funzione per configurare i parametri del motore stepper
def configure_motor_parameters(speed, acc, delay):
    global motor_speed, motor_acc, motor_delay
    motor_speed = speed
    motor_acc = acc
    motor_delay = delay
    print(f"Configurazione motori aggiornata - Velocità: {motor_speed}, Microstep: {motor_acc}, Delay: {motor_delay}")

def receive_http_data():
    while True:  # Ciclo per continuare finché non si ottengono dati validi
        try:
            # Invio richiesta GET
            response = urequests.get("http://" + ip + "/op=withdraw_z")
            
            # Prova a convertire la risposta in un intero
            response_value = int(response.text.strip())
            response.close()
            
            print(f"Dati ricevuti via HTTP: {response_value}")
            return response_value  # Esce dal ciclo restituendo i dati
        except Exception as e:
            print("Errore nel ricevere i dati via HTTP:", e)
            if 'response' in locals():
                try:
                    response.close()  # Chiude la connessione se aperta
                except:
                    pass
            time.sleep(1)  # Attende 1 secondo prima di riprovare


# Funzione per gestire il movimento degli stepper in base al finecorsa
def execute_stepper_movement():

    # Movimento indietro fino al finecorsa
    while True:
        step_motor(motor1_step, motor1_dir, 1, 0, motor_delay)  # Motore 1 indietro
        step_motor(motor2_step, motor2_dir, 1, 1, motor_delay)  # Motore 2 avanti
        if check_limit_switches():
            print("Finecorsa premuto, invertendo direzione.")
            break

    # Movimento avanti e conteggio dei passi
    while True:
        step_motor(motor1_step, motor1_dir, 1, 1, motor_delay)  # Motore 1 avanti
        step_motor(motor2_step, motor2_dir, 1, 0, motor_delay)  # Motore 2 indietro
        if not check_limit_switches():
            break

    # Continua il movimento e conteggia i passi fino a un altro finecorsa
    while True:
        step_motor(motor1_step, motor1_dir, 1, 1, motor_delay)  # Motore 1 avanti
        step_motor(motor2_step, motor2_dir, 1, 0, motor_delay)  # Motore 2 indietro
        primo_counter += 1
        print(primo_counter)
        if check_limit_switches():
            break

    while True:
        step_motor(motor1_step, motor1_dir, 1, 0, motor_delay)  # Motore 1 avanti
        step_motor(motor2_step, motor2_dir, 1, 1, motor_delay)  # Motore 2 indietro
        primo_counter -= 1
        print(primo_counter)
        if not check_limit_switches():
            break

    print(f"Primo Counter aggiornato: {primo_counter}")

    # Calcolo del tempo totale per fare gli step di primo_counter
    total_time = primo_counter * motor_delay
    print(f"Tempo totale per fare gli step di Primo Counter: {total_time} cicli")
    return_to_zero(primo_counter)

# Funzione per tornare alla posizione 0 del primoCounter
def return_to_zero(encoder):
    print("Tornando alla posizione 0 del primoCounter...")
    steps = abs(int(encoder))

    # Movimento per tornare alla posizione 0
    for _ in range(steps):
        step_motor(motor1_step, motor1_dir, 1, 0, motor_delay)
        step_motor(motor2_step, motor2_dir, 1, 1, motor_delay)
    
    print("Posizione 0 raggiunta.")

# Funzione per muovere alla posizione calcolata

def move_to_position(value):
    # Calcola la posizione target
    position = (primo_counter / 16) * ((value * 2) - 1)
    print(f"Calcolando la posizione target: {position}")

    # Determina il numero di passi e la direzione
    steps = abs(int(position))

    # Movimento dei motori verso la posizione calcolata
    for _ in range(steps):
        step_motor(motor1_step, motor1_dir, 1, 0, motor_delay)
        step_motor(motor2_step, motor2_dir, 1, 1, motor_delay)
    print(f"Motori spostati di {steps} passi verso la posizione target.")

    # Invio conferma al server
    send_http_success(steps)

# Funzione per inviare una conferma HTTP con valore "1"
def send_http_success(encoder):
    try:
        # Costruzione URL
        url = f"http://" + ip + "/op=success_x"
        
        # Invio richiesta HTTP POST
        response = urequests.post(url, json={"success": "1"})
        
        # Verifica della risposta
        if response.status_code == 200:  # Controlla se la risposta è valida
            try:
                response_value = int(response.text.strip())  # Converte il testo in un intero
                print(f"Dati ricevuti via HTTP per return_to_zero: {response_value}")
                
                if response_value == 1:
                    return_to_zero(encoder)
                else:
                    print("Risposta diversa da 1")
            except ValueError:
                print("Errore: la risposta non è un intero valido")
        else:
            print(f"Errore HTTP: codice di stato {response.status_code}")
        
        # Chiudi la connessione
        response.close()
    
    except Exception as e:
        print("Errore nell'invio della conferma HTTP: ", e)

def main_loop():
    while True:  # Simula il loop() di Arduino
        try:
            # Invio richiesta HTTP finché la risposta non è "1"
            if first_time:
                x_speed, x_acc, x_delay = send_http_request_until_success()
                execute_stepper_movement()
                first_time = False
            
            # Muove alla posizione calcolata usando il valore ricevuto
            value = receive_http_data()
            if value is not None:
                move_to_position(value)
            
            while True:
                # Lettura dello stato dei finecorsa
                limit_status = read_tca9554() >> 4

                # Controllo del motore 1
                if not (limit_status & 0x01):
                    message = "Finecorsa 1 premuto. Fermando motore 1."
                    print(message)
                    lcd.clear()
                    lcd.print(message, 0, 0)
                else:
                    step_motor(motor1_step, motor1_dir, 100, 1, motor_delay)
                    lcd.clear()
                    lcd.print("Motore 1 in movimento.", 0, 0)

                # Controllo del motore 2
                if not (limit_status & 0x04):
                    message = "Finecorsa 3 premuto. Fermando motore 2."
                    print(message)
                    lcd.clear()
                    lcd.print(message, 0, 20)
                else:
                    step_motor(motor2_step, motor2_dir, 100, 0, motor_delay)
                    lcd.clear()
                    lcd.print("Motore 2 in movimento.", 0, 20)

                # Controlla se uno qualsiasi dei finecorsa è stato attivato
                if check_limit_switches():
                    message = "Un finecorsa è stato attivato. Fermando i motori."
                    print(message)
                    lcd.clear()
                    lcd.print(message, 0, 40)
                    break  # Esce dal ciclo interno per riprendere il loop principale

        except KeyboardInterrupt:
            # Gestione dell'interruzione manuale
            message = "Programma interrotto manualmente."
            print(message)
            lcd.clear()
            lcd.print(message, 0, 0)
            break  # Interrompe il ciclo principale

        except Exception as e:
            # Gestione di eventuali errori imprevisti
            print(f"Errore: {e}")
            lcd.clear()
            lcd.print("Errore nel programma.", 0, 0)
            asyncio.sleep(1)  # Pausa prima di riprendere

# Avvio del loop principale con asyncio
if __name__ == "__main__":
    asyncio.run(main_loop())



