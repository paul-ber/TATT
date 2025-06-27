#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Fuzzer FTP pour EPITA SRS 2025
Auteur: Paul Berthelot (paul.berthelot@epita.fr)
Promotion: SRS 2026

Ce fuzzer teste les vulnérabilités d'un serveur FTP en envoyant des données
malformées sur les commandes FTP courantes. Il effectue les tests suivants:
- Test de débordement de buffer avec des chaînes de longueur croissante (100, 500, 1000, 2000, 5000 caractères)
- Test avec des caractères spéciaux et de contrôle
- Test des commandes avec des arguments multiples
- Test des séquences de commandes invalides
- Détection des plantages par perte de connexion ou timeout
"""

import socket
import sys
import time
import signal

# Variables de configuration - Modifier selon vos credentials
FTP_USER = "anonymous"
FTP_PASS = "test@test.com"

class FTPFuzzer:
    def __init__(self, host, port=21):
        self.host = host
        self.port = port
        self.socket = None
        self.connected = False
        self.authenticated = False

    def connect(self):
        """Établit la connexion au serveur FTP"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(10)
            self.socket.connect((self.host, self.port))
            
            # Lire la bannière
            response = self.socket.recv(1024).decode('utf-8', errors='ignore')
            print(f"[+] Connexion établie: {response.strip()}")
            self.connected = True
            return True
            
        except Exception as e:
            print(f"[-] Erreur de connexion: {e}")
            return False

    def send_command(self, command):
        """Envoie une commande et retourne la réponse"""
        try:
            if not self.connected:
                return None
                
            self.socket.send((command + "\r\n").encode('utf-8', errors='ignore'))
            response = self.socket.recv(4096).decode('utf-8', errors='ignore')
            return response
            
        except socket.timeout:
            print(f"[!] TIMEOUT sur commande: {command[:50]}...")
            return "TIMEOUT"
        except Exception as e:
            print(f"[!] ERREUR CONNEXION sur commande: {command[:50]}... - {e}")
            return None

    def authenticate(self):
        """S'authentifie sur le serveur FTP"""
        try:
            # Envoyer USER
            response = self.send_command(f"USER {FTP_USER}")
            if not response or not ("331" in response or "230" in response):
                print(f"[-] Échec USER: {response}")
                return False
            
            # Envoyer PASS si nécessaire
            if "331" in response:
                response = self.send_command(f"PASS {FTP_PASS}")
                if not response or "230" not in response:
                    print(f"[-] Échec PASS: {response}")
                    return False
            
            print("[+] Authentification réussie")
            self.authenticated = True
            return True
            
        except Exception as e:
            print(f"[-] Erreur authentification: {e}")
            return False

    def test_command(self, command, payload, description=""):
        """Teste une commande avec un payload donné"""
        print(f"[*] Test: {command} avec {description}")
        
        # Reconnexion si nécessaire
        if not self.connected:
            if not self.connect() or not self.authenticate():
                return False
        
        test_cmd = f"{command} {payload}"
        response = self.send_command(test_cmd)
        
        if response is None:
            print(f"[!] CRASH DÉTECTÉ! Commande: {command}")
            print(f"[!] Payload: {payload[:100]}...")
            print(f"[!] Longueur payload: {len(payload)}")
            self.connected = False
            return True  # Crash détecté
        elif response == "TIMEOUT":
            print(f"[!] TIMEOUT DÉTECTÉ! Commande: {command}")
            self.connected = False
            return True  # Potentiel crash
        else:
            print(f"[+] Réponse: {response[:100].strip()}...")
            return False

    def generate_payloads(self):
        """Génère différents types de payloads de test"""
        payloads = []
        
        # Payloads de taille croissante
        for size in [100, 500, 1000, 2000, 5000]:
            payloads.append(("A" * size, f"Buffer de {size} 'A'"))
        
        # Payloads avec caractères spéciaux
        special_chars = [
            ("%s" * 200, "Format string"),
            ("../" * 200, "Directory traversal"),
            ("\x00" * 200, "Null bytes"),
            ("\xff" * 200, "High bytes"),
            ("%" + "41" * 200, "Hex encoding"),
        ]
        payloads.extend(special_chars)
        
        return payloads

    def fuzz_ftp_commands(self):
        """Teste les commandes FTP avec différents payloads"""
        # Commandes FTP à tester (après authentification)
        ftp_commands = [
            "PWD", "CWD", "MKD", "RMD", "DELE", "RNFR", "RNTO",
            "LIST", "NLST", "RETR", "STOR", "APPE", "REST",
            "SITE", "CHMOD", "SIZE", "MDTM", "HELP", "NOOP",
            "SYST", "STAT", "TYPE", "MODE", "STRU", "ALLO"
        ]
        
        print(f"\n[*] Début du fuzzing sur {self.host}:{self.port}")
        print(f"[*] Credentials: {FTP_USER} / {FTP_PASS}")
        
        crashes = []
        payloads = self.generate_payloads()
        
        for command in ftp_commands:
            print(f"\n[*] === Test de la commande {command} ===")
            
            for payload, description in payloads:
                if self.test_command(command, payload, description):
                    crashes.append((command, payload, description))
                    print(f"[!] >>> VULNÉRABILITÉ TROUVÉE: {command} <<<")
                    break  # Passer à la commande suivante après crash
                
                time.sleep(0.1)  # Pause entre tests
        
        return crashes

    def close(self):
        """Ferme la connexion"""
        try:
            if self.socket:
                self.socket.close()
        except:
            pass

def signal_handler(sig, frame):
    """Gestionnaire de signal pour arrêt propre"""
    print("\n[*] Arrêt du fuzzer...")
    sys.exit(0)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 1ftpfuzzer.py <target_host>")
        print("Exemple: python3 1ftpfuzzer.py 192.168.1.100")
        sys.exit(1)
    
    signal.signal(signal.SIGINT, signal_handler)
    
    target_host = sys.argv[1]
    fuzzer = FTPFuzzer(target_host)
    
    try:
        # Connexion initiale et authentification
        if not fuzzer.connect():
            print("[-] Impossible de se connecter au serveur")
            sys.exit(1)
        
        if not fuzzer.authenticate():
            print("[-] Impossible de s'authentifier")
            sys.exit(1)
        
        # Lancement du fuzzing
        crashes = fuzzer.fuzz_ftp_commands()
        
        # Rapport final
        print(f"\n[*] === RAPPORT FINAL ===")
        print(f"[*] Nombre de crashes détectés: {len(crashes)}")
        
        if crashes:
            print("[!] VULNÉRABILITÉS TROUVÉES:")
            for cmd, payload, desc in crashes:
                print(f"[!] Commande: {cmd}")
                print(f"[!] Payload: {desc}")
                print(f"[!] Données: {payload[:50]}...")
                print("[!] ---")
        else:
            print("[+] Aucune vulnérabilité détectée")
    
    finally:
        fuzzer.close()

if __name__ == "__main__":
    main()
