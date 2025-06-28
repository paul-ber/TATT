#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
TATT - DM3 : Fuzzer FTP
Auteur: Paul Berthelot (paul.berthelot@epita.fr)
Promotion: SRS 2026

Ce fuzzer teste les vulnérabilités d'un serveur FTP en envoyant des données
malformées sur toutes les commandes FTP. Il effectue les tests suivants:
- Test de débordement de buffer avec des chaînes de longueur croissante (100,
  500, 1000, 2000, 5000 caractères)
- Test avec des caractères spéciaux et de contrôle
- Test des commandes avec des arguments multiples
- Détection des plantages par perte de connexion ou timeout

Nouvelle connexion pour chaque test individuel afin d'éviter les effets de bord
et d'avoir un processus serveur "propre" pour chaque tentative d'exploitation.

EXEMPLES DE SORTIE DU FUZZER :

Test négatif (aucune vulnérabilité détectée) :
[*] Test: PWD avec Buffer de 100 'A'
[+] Connexion établie: 220 Welcome to FuzzeMeSteckFTP v1.0
[+] Authentification réussie
[+] Réponse: 501 Syntax error in parameters or arguments...
[+] Test terminé normalement
[...]

Test positif (vulnérabilité détectée) :
[*] Test: TYPE avec Buffer de 500 'A'
[+] Connexion établie: 220 Welcome to FuzzeMeSteckFTP v1.0
[+] Authentification réussie
[!] CRASH DÉTECTÉ! Aucune réponse reçue
[!] Commande: TYPE
[!] Payload: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA[...]
[!] Longueur payload: 500
[!] >>> VULNÉRABILITÉ TROUVÉE: TYPE <<<
[...]

[*] === RAPPORT FINAL ===
[*] Nombre de crashes détectés: X
[!] VULNÉRABILITÉS TROUVÉES:
[!] Commande: TYPE
[!] Payload: Buffer de 500 'A'
[!] Données: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA[...]
[!] ---
[...]
"""

import socket
import sys
import time
import signal

# Variables d'authentification
FTP_USER = "test"
FTP_PASS = "test"

class FTPFuzzer:
    def __init__(self, host, port=21):
        self.host = host
        self.port = port

    def connect_and_auth(self):
        """Établit une nouvelle connexion et s'authentifie"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(10)
            sock.connect((self.host, self.port))

            # Lire la bannière
            response = sock.recv(1024).decode('utf-8', errors='ignore')
            print(f"[+] Connexion établie: {response.strip()}")

            # Authentification
            sock.send(f"USER {FTP_USER}\r\n".encode('utf-8'))
            response = sock.recv(1024).decode('utf-8', errors='ignore')
            if not ("331" in response or "230" in response):
                print(f"[-] Échec USER: {response}")
                sock.close()
                return None

            if "331" in response:
                sock.send(f"PASS {FTP_PASS}\r\n".encode('utf-8'))
                response = sock.recv(1024).decode('utf-8', errors='ignore')
                if "230" not in response:
                    print(f"[-] Échec PASS: {response}")
                    sock.close()
                    return None

            print("[+] Authentification réussie")
            return sock

        except Exception as e:
            print(f"[-] Erreur de connexion/auth: {e}")
            return None

    def test_single_command(self, command, payload, description=""):
        """Teste UNE commande avec UNE connexion fraîche"""
        print(f"\n[*] Test: {command} avec {description}")

        # Nouvelle connexion pour ce test
        sock = self.connect_and_auth()
        if not sock:
            print("[-] Impossible de se connecter pour ce test")
            return False

        try:
            # Envoyer la commande de test
            test_cmd = f"{command} {payload}\r\n"
            sock.send(test_cmd.encode('utf-8', errors='ignore'))

            # Attendre la réponse avec un timeout
            sock.settimeout(5)
            response = sock.recv(4096).decode('utf-8', errors='ignore')

            if response:
                print(f"[+] Réponse: {response[:100].strip()}...")
                print("[+] Test terminé normalement")
                crash_detected = False
            else:
                print("[!] CRASH DÉTECTÉ! Aucune réponse reçue")
                crash_detected = True

        except socket.timeout:
            print("[!] CRASH DÉTECTÉ! Timeout - serveur ne répond plus")
            crash_detected = True
        except ConnectionResetError:
            print("[!] CRASH DÉTECTÉ! Connexion fermée brutalement")
            crash_detected = True
        except Exception as e:
            print(f"[!] CRASH DÉTECTÉ! Erreur: {e}")
            crash_detected = True
        finally:
            try:
                sock.close()
            except:
                pass

        if crash_detected:
            print(f"[!] Commande: {command}")
            print(f"[!] Payload: {payload[:100]}...")
            print(f"[!] Longueur payload: {len(payload)}")
            return True

        return False

    def generate_payloads(self):
        """Génère différents types de payloads de test"""
        payloads = []

        # Payloads de taille croissante
        for size in [100, 500, 1000, 2000, 5000]:
            payloads.append(("A" * size, f"Buffer de {size} 'A'"))

        # Payloads avec caractères spéciaux
        special_chars = [
            ("%s" * 10, "Format string"),
            ("../" * 10, "Directory traversal"),
            ("\x00" * 10, "Null bytes"),
            ("\xff" * 10, "High bytes"),
            ("%" + "41" * 10, "Hex encoding"),
        ]
        payloads.extend(special_chars)

        return payloads

    def fuzz_ftp_commands(self):
        """Teste les commandes FTP avec différents payloads"""
        # Commandes FTP à tester

        ftp_commands = [
            "ABOR", "ACCT", "ADAT", "ALLO", "APPE", "AUTH", "AVBL", "CCC",
            "CDUP", "CONF", "CSID", "CWD", "DELE", "DSIZ", "ENC", "EPRT",
            "EPSV", "FEAT", "HELP", "HOST", "LANG", "LIST", "LPRT", "LPSV",
            "MDTM", "MFCT", "MFF", "MFMT", "MIC", "MKD", "MLSD", "MLST",
            "MODE", "NLST", "NOOP", "OPTS", "PASS", "PASV", "PBSZ", "PORT",
            "PROT", "PWD", "QUIT", "REIN", "REST", "RETR", "RMD", "RMDA",
            "RNFR", "RNTO", "SITE", "SIZE", "SMNT", "SPSV", "STAT", "STOR",
            "STOU", "STRU", "SYST", "THMB", "TYPE", "USER", "XCUP", "XMKD",
            "XPWD", "XRCP", "XRMD", "XRSQ", "XSEM", "XSEN"
        ]

        print(f"[*] Début du fuzzing sur {self.host}:{self.port}")
        print(f"[*] Credentials: {FTP_USER} / {FTP_PASS}")
        print(f"[*] Stratégie: Connexion fraîche pour chaque test")

        crashes = []
        payloads = self.generate_payloads()

        for command in ftp_commands:
            print(f"\n[*] === Test de la commande {command} ===")

            for payload, description in payloads:
                crash_detected = self.test_single_command(command, payload, description)

                if crash_detected:
                    crashes.append((command, payload, description))
                    print(f"[!] >>> VULNÉRABILITÉ TROUVÉE: {command} <<<")

                time.sleep(0.5)

        return crashes

def signal_handler(sig, frame):
    """Gestionnaire de signal pour arrêt propre"""
    print("\n[*] Arrêt du fuzzer...")
    sys.exit(0)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 1ftpfuzzer.py <target_host>")
        print("Exemple: python3 1ftpfuzzer.py 127.0.0.1")
        sys.exit(1)

    signal.signal(signal.SIGINT, signal_handler)

    target_host = sys.argv[1]
    fuzzer = FTPFuzzer(target_host)

    try:
        # Test de connexion initial
        print("[*] Test de connexion initial...")
        test_sock = fuzzer.connect_and_auth()
        if not test_sock:
            print("[-] Impossible de se connecter au serveur")
            sys.exit(1)
        test_sock.close()
        print("[+] Serveur accessible, début du fuzzing...")

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

    except KeyboardInterrupt:
        print("\n[*] Arrêt demandé par l'utilisateur")
    except Exception as e:
        print(f"[-] Erreur inattendue: {e}")

if __name__ == "__main__":
    main()
