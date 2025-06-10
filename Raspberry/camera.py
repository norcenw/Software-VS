import cv2
import time
import numpy as np
import requests
from pyzbar.pyzbar import decode

# URL di destinazione
URL = "http://vertistock.local/op=qr1"

# Variabili per throttling sullo stesso QR
last_qr = ""
last_sent_time = 0
THROTTLE_SECONDS = 30

def send_qr(qr_data: str) -> bool:
    """
    Invia una POST con qr_data.
    Applica un throttle di THROTTLE_SECONDS sullo stesso qr_data.
    Ritorna True se invio effettuato con status 2xx, False altrimenti o se throttled.
    """
    global last_qr, last_sent_time
    now = time.time()

    # Throttle: evita invii ripetuti più volte in pochi secondi
    if qr_data == last_qr and (now - last_sent_time) < THROTTLE_SECONDS:
        print(f"[THROTTLE] '{qr_data}' già inviato da meno di {THROTTLE_SECONDS}s.")
        return False

    last_qr = qr_data
    last_sent_time = now

    try:
        resp = requests.post(URL, data=qr_data, timeout=5)
        print(f"[HTTP {resp.status_code}] {qr_data}")
        return 200 <= resp.status_code < 300
    except requests.RequestException as e:
        print(f"[ERROR] Invio HTTP fallito: {e}")
        return False

def main():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Errore: impossibile aprire la camera")
        return

    # Impostazione fullscreen
    win = "QR Scanner"
    cv2.namedWindow(win, cv2.WND_PROP_FULLSCREEN)
    cv2.setWindowProperty(win, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("Errore: frame non disponibile")
                break

            # Grayscale per decode più veloce
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            qr_list = decode(gray)

            # 1) Maschera dei QR (255 dentro ogni rettangolo, 0 altrove)
            mask = np.zeros(frame.shape[:2], dtype=np.uint8)
            for qr in qr_list:
                x, y, w, h = qr.rect
                mask[y:y+h, x:x+w] = 255
            mask_3 = cv2.merge([mask, mask, mask])

            # 2) Blur pesante + fusione con bianco (effetto schiarito)
            blurred = cv2.GaussianBlur(frame, (51, 51), 0)
            white   = np.full(frame.shape, 255, dtype=np.uint8)
            blurred_light = cv2.addWeighted(blurred, 0.5, white, 0.5, 0)

            # 3) Composizione: sfondo offuscato+chiaro, QR nitidi
            inv_mask = cv2.bitwise_not(mask)
            inv_3    = cv2.merge([inv_mask, inv_mask, inv_mask])
            background = cv2.bitwise_and(blurred_light, inv_3)
            foreground = cv2.bitwise_and(frame, mask_3)
            final = cv2.add(background, foreground)

            # 4) Overlay: invio dati e disegno rettangolo + testo multilinea
            for qr in qr_list:
                data = qr.data.decode('utf-8')
                x, y, w, h = qr.rect

                success = send_qr(data)
                # Verde se inviato o throttled, rosso se errore HTTP
                color = (0, 255, 0) if success or data == last_qr else (0, 0, 255)

                # Disegna il rettangolo
                cv2.rectangle(final, (x, y), (x + w, y + h), color, 2)

                # Prepara il testo viola, piccolo
                font       = cv2.FONT_HERSHEY_SIMPLEX
                font_scale = 0.4
                thickness  = 1
                purple     = (255, 0, 255)

                # Trasforma "{name: A, code: B, count: C}" in ["name: A", "code: B", "count: C"]
                entries = data.strip("{}").split(",")
                lines = [e.strip() for e in entries]

                # Posiziona le righe appena sotto il QR
                text_x = x
                text_y = y + h + 15
                line_height = 15

                for i, line in enumerate(lines):
                    y_i = text_y + i * line_height
                    cv2.putText(final, line, (text_x, y_i),
                                font, font_scale, purple, thickness)

            cv2.imshow(win, final)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    finally:
        cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
