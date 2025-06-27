#!/bin/bash
for i in $(seq 1 20); do
  echo "ABCD%$i\$x" > banner.txt
  echo "Test avec offset $i"
  ./my_ftpd -v 1 -u users.txt  # Assurez-vous que -v 1 est utilisé pour activer l'affichage de la bannière
done
