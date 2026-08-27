NUCLEO-STM32L476RG wireless sensor network Internship Project 

Ce projet réalisé dans le cadre d'un stage propose une solution de communication d'informations entre cartes STM32L476RG organisées en réseau.
Le programme dans sa version la plus récente implémente des fonctions non testées qui tiennent compte de la consommation théorique du noeud et de ses voisins afin d'adapter son fonctionnement.
Le système testé intègre deux modules :

-  Un dispositif de communication série Half Duplex HC-12 SI4463 émetteur-récepteur sans fil
-  Un Capteur de température et de pression BMP280 communiquant ses données de mesure via protocole I2C

Fonctionnalités notables : 

- Trame personalisée d'envoi des messages de la forme <img width="1137" height="172" alt="image" src="https://github.com/user-attachments/assets/4b52e4a6-45e3-4275-805d-63bc157e68e8" />
- Communication entre capteur sur différents slots temporels
- Attribution d'un gradient permettant de situer la proximité d'un noeud-capteur à l'origine
- Code de Hamming (8,4) correcteur d'erreur sur chaque 4 bits de données de la trame 

Guide de fonctionnement et d'installation : 

Afin de tester le fonctionnement du projet il est nécessaire d'avoir à disposition au moins 2 exemplaires de cartes **STM32L476RG**
ainsi que les deux modules cités plus haut **HC-12 SI4463 émetteur-récepteur** sans fil et **BMP280**

Réaliser le cablâge suivant si le shield n'est pas à disposition 
<img width="1499" height="857" alt="image" src="https://github.com/user-attachments/assets/b95eee3e-da49-4c11-ad21-8b118090cc86" />

Pour reprendre le travail à partir de ce dossier, est fourni ci dessous un guide d'installation 
Installer les logiciels STM32CubeProgrammer(https://www.st.com/en/development-tools/stm32cubeprog.html), 
STM32CubeMX (https://www.st.com/en/development-tools/stm32cubemx.html), 
PuTTY ou autre application offrant un terminal et permettant une liaison série pour le debug auquel cas VScode peut être conseillé pour répliquer l'environnement de dévellopement initial, 
dans le cas ou STM32CubeIDE est choisi, PuTTY et VScode ne sont plus nécessaires car l'application devrait proposer des alternatives mais cela reste à vérifier donc je ne peux pas le recommander

Une fois les logiciels installés, dans le cas de communication entre 2 cartes, générer le fichier main.c puis téleverser le code sur la carte via cubeProgrammer => open file =>  interruptions/build/debug/interruptions.elf
Répéter l'opération pour la deuxieme carte en générant le fichier noeud_central.c, sur VScode on réalise cela en copiant le contenu du fichier et en le remplaçant par celui de main.c
Ouvrir ensuite PuTTY et sélectionner les ports séries correspondants aux 2 cartes (obtenable via gestionnaire des périphériques) sélectionner le baudrate 9600 
appuyer sur le bouton reset des cartes ( la carte ayant le code du noeud central devrait toujours être activée en dernier car c'est elle qui commence la procédure d'attribution du gradient)
Observer finalement les résultats des tests via le terminal PuTTY
Dans le cas de plus de 2 cartes, répéter la première opération pour téleverser le fichier main.c mais en modifiant pour chaque carte la constante uint8_t node_addr désignant l'adresse de la carte. 
