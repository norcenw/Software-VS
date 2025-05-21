import cv2
import requests
from pyzbar.pyzbar import decode

# Endpoint di destinazione
URL = "http://vertistock.local/op=qr1"

# Insieme dei QR già processati con successo
processed = set()

def send_qr(qr_data):
    """
    Invia una POST con il payload qr_data.
    Ritorna True se la risposta ha status_code 2xx, altrimenti False.
    """
    try:
        resp = requests.post(URL, data=qr_data, timeout=5)
        print(f"[HTTP {resp.status_code}] {qr_data}")
        return 200 <= resp.status_code < 300
    except requests.RequestException as e:
        print(f"Errore HTTP: {e}")
        return False

def main():
    # Apre la webcam (su Raspberry Pi spesso è /dev/video0)
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Errore: impossibile aprire la camera")
        return

    # Configura la finestra a pieno schermo
    window_name = "QR Scanner"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("Errore: impossibile leggere frame")
                break

            # Conversione in scala di grigi per velocizzare decode
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

            # Decodifica dei QR code
            for qr in decode(gray):
                qr_data = qr.data.decode('utf-8')
                x, y, w, h = qr.rect

                # Se non è già stato processato con successo, provo a inviare
                if qr_data not in processed:
                    success = send_qr(qr_data)
                    if success:
                        processed.add(qr_data)

                # Disegna il riquadro e il testo
                color = (0, 255, 0) if qr_data in processed else (0, 0, 255)
                cv2.rectangle(frame, (x, y), (x + w, y + h), color, 2)
                cv2.putText(frame, qr_data, (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

            # Mostra il frame
            cv2.imshow(window_name, frame)

            # Esci con 'q'
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    finally:
        cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
