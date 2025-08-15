import serial
import re
import json

def parse_line(line):
    """Extrahiert den Schlüssel und den Wert aus einer Zeile, entfernt Einheiten."""
    match = re.match(r'([^:]+):\s*(-?\d+(?:\.\d+)?)\s?[A-Za-z%°VAm]*$', line)  # Entfernt Einheiten
    if match:
        key = match.group(1).strip().replace(" ", "_")
        value = match.group(2).strip()
        return key, value
    
    match_text = re.match(r'([^:]+):\s*(.*)', line)  # Falls es sich um einen Textwert handelt
    if match_text:
        key = match_text.group(1).strip().replace(" ", "_")
        value = match_text.group(2).strip()
        return key, value
    
    return None, None

def main():
    try:
        # Serielle Schnittstelle des USB-Geräts
        usb_serial = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=1)
        # Serielle Schnittstelle zum Weiterleiten
        pi_serial = serial.Serial('/dev/serial0', baudrate=115200, timeout=1)
    except serial.SerialException as e:
        print("Fehler beim Öffnen der seriellen Schnittstelle:", e)
        return
    
    data_dict = {}
    error_section = False  # Flag, um den zweiten Abschnitt nach ############################## zu erfassen
    
    while True:
        try:
            line = usb_serial.readline().decode('utf-8', errors='ignore').strip()
            if not line:
                continue
            print("Empfangen:", line)
            
            if "##############################" in line:
                if error_section:
                    # Datensatz vollständig, senden
                    if data_dict:
                        json_output = json.dumps(data_dict)
                        pi_serial.write((json_output + "\n").encode('utf-8'))
                        print("Gesendet:", json_output)
                        data_dict.clear()
                    error_section = False  # Zurücksetzen für nächsten Durchlauf
                else:
                    error_section = True  # Aktivieren, um Fehler-Meldungen zu erfassen
                continue
            
            key, value = parse_line(line)
            if key and value:
                data_dict[key] = value  # Speichert sowohl Zahlen als auch Textwerte
        
        except Exception as e:
            print("Fehler während der Verarbeitung:", e)
            continue

if __name__ == "__main__":
    main()
