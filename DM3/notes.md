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

Avec le script de fuzzing, on peut voir qu'il y a un crash lors de la commande TYPE avec un grand payload (1000 octets par exemple).

On voit en effet dans gdb que le thread segfault.