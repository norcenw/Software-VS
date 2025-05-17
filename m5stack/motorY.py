import os, sys, io
import M5
from M5 import lcd
from machine import Pin, I2C
import time
import uasyncio as asyncio
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

# Configurazione dei pin per il motore stepper
MOTOR_DIR_PIN = 19
MOTOR_STEP_PIN = 18

#definizione primo_counter
primo_counter = 0

# Impostazione dei pin del motore
motor_dir = Pin(MOTOR_DIR_PIN, Pin.OUT)
motor_step = Pin(MOTOR_STEP_PIN, Pin.OUT)
y_speed = 100
y_acc = 1
y_delay = 0.001

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

# Funzione per verificare lo stato dei finecorsa
def check_limit_switches():
    """
    Restituisce True se uno qualsiasi dei finecorsa è premuto.
    """
    status = read_tca9554()
    # I primi 4 bit rappresentano gli ingressi dei finecorsa
    return (status & 0xF0) != 0xF0

# Funzione per inviare richiesta HTTP finché la risposta non è "1"
def send_http_request_until_success():
    while True:
        try:
            response = urequests.get("http://" + ip + "/op=start_y")
            response_json = response.json()
            response.close()
            
            if response_json.get("status") == 1:
                y_speed = response_json.get("yspeed", 100)  # Velocità del motore stepper (default 100)
                y_acc = response_json.get("yacc", 1)  # Microstep (default 1)
                y_delay = response_json.get("ydelay", 0.001)  # Delay (default 0.001)
                print(f"Risposta HTTP ricevuta. Velocità: {y_speed}, Microstep: {y_acc}, Delay: {y_delay}")
                configure_motor_parameters(y_speed, y_acc, y_delay)
                return y_speed, y_acc, y_delay
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

# Funzione per ricevere dati HTTP e fare il parseInt del valore ricevuto
async def receive_http_data():
    while True:  # Ciclo per continuare finché non si ottengono dati validi
        try:
            # Invio richiesta GET
            response = urequests.get("http://" + ip + "/op=withdraw_y")
            
            # Prova a convertire la risposta in un intero
            response_value = int(response.text.strip())
            response.close()
            
            print(f"Dati ricevuti via HTTP: {response_value}")
            return response_value  # Esce dal ciclo restituendo i dati
        except Exception as e:
            print("Errore nel ricevere i dati via HTTP: ", e)
            await asyncio.sleep(1)  # Attende prima di riprovare  

# Funzione per gestire il movimento dello stepper in base al finecorsa
def execute_stepper_movement():
    primo_counter = 0
    secondo_counter = 0

    # Movimento indietro fino al finecorsa
    while True:
        step_motor(motor_step, motor_dir, 1, 0, motor_delay)  # Motore indietro
        if check_limit_switches():
            print("Finecorsa premuto, invertendo direzione.")
            break

    # Movimento avanti e conteggio dei passi
    while True:
        step_motor(motor_step, motor_dir, 1, 1, motor_delay)  # Motore avanti
        primo_counter += 1
        if not check_limit_switches():
            break

    # Continua il movimento e conteggia i passi fino a un altro finecorsa
    while True:
        step_motor(motor_step, motor_dir, 1, 1, motor_delay)  # Motore avanti
        secondo_counter += 1
        if check_limit_switches():
            break

    # Ferma il conteggio dei passi al rilascio del finecorsa
    while check_limit_switches():
        step_motor(motor_step, motor_dir, 1, 1, motor_delay)  # Motore avanti
        secondo_counter += 1

    # Reinizializza il primoCounter
    primo_counter = primo_counter - secondo_counter
    print(f"Primo Counter aggiornato: {primo_counter}")

    # Calcolo del tempo totale per fare gli step di primo_counter
    total_time = primo_counter * motor_delay
    print(f"Tempo totale per fare gli step di Primo Counter: {total_time} secondi")

# Funzione per tornare alla posizione 0 del primoCounter
def return_to_zero(primo_counter):
    print("Tornando alla posizione 0 del primoCounter...")
    direction = 0 if primo_counter > 0 else 1  # Determina la direzione per tornare indietro
    steps = abs(primo_counter)

    # Movimento per tornare alla posizione 0
    for _ in range(steps):
        step_motor(motor_step, motor_dir, 1, direction, motor_delay)
    
    print("Posizione 0 raggiunta.")

# Funzione per muovere alla posizione calcolata
def move_to_position(value):
    # Calcola la posizione target
    position = (primo_counter / 16) * ((value * 2) - 1)
    print(f"Calcolando la posizione target: {position}")

    # Determina il numero di passi e la direzione
    steps = abs(int(position))
    direction = 0 if position < 0 else 1

    # Movimento del motore verso la posizione calcolata
    step_motor(motor_step, motor_dir, steps, direction, motor_delay)
    print(f"Motore spostato di {steps} passi verso la posizione target.")

    # Invio conferma al server
    send_http_success()

# Funzione per inviare una conferma HTTP con valore "1"
def send_http_success():
    try:
        # Costruzione URL
        url = f"http://" + ip + "/op=success_y"
        
        # Invio richiesta HTTP POST
        response = urequests.post(url, json={"success": "1"})
        
        # Verifica della risposta
        if response.status_code == 200:  # Controlla se la risposta è valida
            try:
                response_value = int(response.text.strip())  # Converte il testo in un intero
                print(f"Dati ricevuti via HTTP per return_to_zero: {response_value}")
                
                if response_value == 1:
                    return_to_zero(primo_counter)
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

async def main_loop():
    while True:  # Simula il loop() di Arduino
        try:
            # Invio richiesta HTTP finché la risposta non è "1"
            y_speed, y_acc, y_delay = await send_http_request_until_success()
            await execute_stepper_movement()
            
            # Muove alla posizione calcolata usando il valore ricevuto
            value = await receive_http_data()
            if value is not None:
                await move_to_position(value)
            
            while True:
                # Lettura dello stato dei finecorsa
                limit_status = await read_tca9554() >> 4

                # Controllo del motore
                if not (limit_status & 0x01):  # Finecorsa 1 premuto
                    message = "Finecorsa 1 premuto. Fermando il motore."
                    print(message)
                    lcd.clear()
                    lcd.print(message, 0, 0)
                else:
                    await step_motor(motor_step, motor_dir, 100, 1, motor_delay)
                    lcd.clear()
                    lcd.print("Motore in movimento.", 0, 0)

                # Controlla se uno qualsiasi dei finecorsa è stato attivato
                if await check_limit_switches():
                    message = "Finecorsa attivato. Fermando il motore."
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
            await asyncio.sleep(1)  # Pausa prima di riprendere

# Avvio del loop principale con asyncio
if __name__ == "__main__":
    asyncio.run(main_loop())