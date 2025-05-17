import cv2
import time
import requests
from pyzbar.pyzbar import decode

# Variabili per memorizzare l'ultimo QR code inviato e il tempo dell'invio
last_qr = ""
last_sent_time = 0

# Apri la webcam (0 per la webcam principale, cambialo se necessario)
cap = cv2.VideoCapture(0)

# Assicura che la webcam sia aperta correttamente
if not cap.isOpened():
    print("Errore: impossibile aprire la webcam")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Errore: impossibile ottenere frame dalla webcam")
        break

    # Decodifica eventuali QR Code presenti nel frame
    qr_codes = decode(frame)
    for qr in qr_codes:
        qr_data = qr.data.decode('utf-8')
        print(f"QR Code rilevato: {qr_data}")

        # Verifica se il QR code rilevato è uguale a quello appena inviato
        current_time = time.time()
        if qr_data == last_qr and (current_time - last_sent_time) < 30:
            print("QR Code identico a quello inviato di recente; attendo 30 secondi per reinvio.")
        else:
            # Aggiorna il buffer e il timestamp
            last_qr = qr_data
            last_sent_time = current_time

            # Invia la richiesta HTTP POST con il contenuto del QR Code
            url = "http://vertistock.local/op=qr2"
            try:
                response = requests.post(url, data=qr_data, timeout=3)
                print(f"Richiesta inviata: {response.status_code}")
            except requests.exceptions.RequestException as e:
                print(f"Errore nell'invio della richiesta: {e}")

        # Disegna un riquadro attorno al QR Code
        (x, y, w, h) = qr.rect
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    # Mostra il frame con la webcam attiva
    cv2.imshow("Webcam QR Scanner", frame)

    # Esci con il tasto 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Rilascia la webcam e chiudi le finestre
cap.release()
cv2.destroyAllWindows()
