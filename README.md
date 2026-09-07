# Réseau de capteurs sans fil — STM32L476RG

Projet réalisé dans le cadre d'un stage assistant ingénieur au **Laboratoire d'Électronique, Antennes et Télécommunications (LEAT)**, au sein de l'équipe **EDGE**, avec le soutien de **DS4H**.

Ce projet porte sur la conception, l'implémentation et l'expérimentation d'un **réseau de capteurs sans fil (WSN)** composé de nœuds autonomes basés sur des cartes **STM32L476RG**.

L'objectif est d'étudier différents mécanismes permettant d'améliorer la **fiabilité**, les **performances** et l'**efficacité énergétique** d'un réseau de capteurs, tout en caractérisant expérimentalement le comportement du système.

---

## Vue d'ensemble

Chaque nœud-capteur est constitué de :

* un microcontrôleur **STM32L476RG** ;
* un module radio **HC-12** basé sur le circuit **Si446x** ;
* un capteur environnemental **BMP280** ;
* une liaison **I²C** pour l'acquisition des mesures ;
* des liaisons série **UART/LPUART** pour la communication avec le module radio et le débogage.

Le réseau permet à plusieurs nœuds de communiquer directement ou indirectement avec un nœud central en utilisant une architecture multi-sauts.

<img width="787" height="451" alt="image" src="https://github.com/user-attachments/assets/4acb7ac9-67a1-4e42-a76d-fa06b968eafb" />


Le prototype a notamment été utilisé pour expérimenter une communication entre **trois nœuds-capteurs**, avec identification des adresses source et destination et relais des messages lorsque cela est nécessaire.

---

## Fonctionnalités

### Communication radio

* Communication sans fil **half-duplex** via HC-12.
* Communication entre plusieurs nœuds.
* Adressage des messages avec source et destination.
* Accusé de réception (**ACK**) et retransmission des messages.
* Mesure du **Round Trip Time (RTT)**.
* Mesure du taux de réception des paquets.
* Possibilité d'utiliser différents niveaux de puissance du HC-12.

### Accès au canal

* Mise en place d'une organisation temporelle de type **TDMA (Time Division Multiple Access)**.
* Attribution de fenêtres temporelles aux différents nœuds.
* Utilisation d'un timer RTC pour maintenir la synchronisation pendant les phases basse consommation.
* Prise en compte des contraintes de réveil liées au mode STOP2.

### Routage par gradient

Le réseau utilise une approche inspirée du **Gradient-Based Routing (GBR)**.

Chaque nœud possède un gradient représentant sa position virtuelle par rapport au nœud central. Les messages peuvent ainsi être relayés vers des nœuds présentant un gradient plus faible afin de converger vers la destination.

Le gradient est attribué lors de la phase d'initialisation du réseau.

### Correction d'erreurs

Un **code de Hamming (8,4)** a été implémenté afin d'ajouter un mécanisme de correction d'erreurs au niveau des données.

Les données sont séparées en groupes de 4 bits puis encodées sous forme d'octets contenant les bits de données et les bits de parité.

Le décodage permet notamment de :

* calculer le syndrome ;
* localiser une erreur sur un bit ;
* corriger l'erreur détectée ;
* restituer les données originales.

Le système conserve également le **CRC intégré au HC-12** comme mécanisme complémentaire de détection d'erreurs.

### Gestion de la consommation

Le système exploite les capacités basse consommation du **STM32L476RG**, notamment le mode **STOP2**.

Entre deux phases de communication, le nœud peut désactiver une grande partie de ses périphériques et être réveillé par les mécanismes compatibles avec ce mode.

Le projet étudie également :

* la consommation des différents modes de fonctionnement ;
* l'impact du temps d'activité du système ;
* la consommation du module radio ;
* le compromis entre puissance d'émission, portée et fiabilité ;
* l'influence du nombre de sauts sur les performances du réseau.

Une fonction de **profilage énergétique des nœuds voisins** a également été développée afin d'étudier une adaptation dynamique du gradient en fonction de l'autonomie restante. Cette fonctionnalité constitue une partie plus récente du développement et n'a pas été validée de manière aussi complète que les fonctionnalités principales.

---

## Format des trames

Le protocole utilise une trame personnalisée structurée autour des champs suivants :

<img width="611" height="107" alt="image" src="https://github.com/user-attachments/assets/fd4c6593-9546-4b66-885f-256e325013ef" />

La séparation du champ **type + séquence** permet notamment d'identifier le type de message et de suivre les échanges entre les nœuds.

---

## Architecture logicielle

Le fonctionnement du nœud repose principalement sur une architecture événementielle utilisant les **interruptions et fonctions callback**.

Le fonctionnement général est le suivant :

<img width="892" height="827" alt="image" src="https://github.com/user-attachments/assets/b7f9a168-be9c-438e-bea2-00b3112c02f9" />


Lorsqu'une trame est reçue, elle est décodée et vérifiée. Le nœud peut alors :

1. calculer le temps d'arrivée lorsqu'il s'agit d'un ACK ;
2. répondre par un ACK ;
3. relayer le message si son gradient permet de rapprocher le message de la destination.

---

## Synchronisation et TDMA

L'implémentation du TDMA a nécessité de prendre en compte les contraintes du mode STOP2.

Les horloges classiques du STM32 étant désactivées dans ce mode, certaines mesures temporelles ne peuvent plus être réalisées de la même manière qu'en fonctionnement normal.

Le projet utilise notamment :

* le **RTC** pour les réveils périodiques ;
* le **DWT / CYCCNT** pour mesurer avec précision certains temps d'exécution et délais internes ;
* le **LPUART1**, compatible avec les mécanismes de réveil utilisés en STOP2.

Un problème de dérive des timers entre les nœuds a notamment été rencontré puis corrigé en utilisant un timer RTC restant actif après le réveil.

---

## Matériel

### STM32L476RG

Le STM32L476RG a été retenu pour :

* ses capacités basse consommation ;
* son mode STOP2 ;
* ses possibilités de réveil ;
* ses périphériques de communication ;
* son niveau de contrôle matériel adapté aux expérimentations.

### HC-12 / Si446x

Le HC-12 constitue la liaison radio du système.

Caractéristiques exploitées dans le projet :

* communication série sans fil ;
* fonctionnement half-duplex ;
* fréquence autour de **433 MHz** ;
* plusieurs niveaux de puissance d'émission ;
* plusieurs modes de fonctionnement ;
* gestion interne des paquets et CRC.

Les essais ont également permis d'étudier le compromis entre **puissance d'émission, portée, consommation et fiabilité**.

### BMP280

Le **BMP280** est utilisé comme capteur environnemental.

Il permet notamment d'acquérir :

* la température ;
* la pression atmosphérique.

La communication avec le STM32 est réalisée via **I²C**.

---

## Installation

### Matériel nécessaire

Pour reproduire une configuration minimale à deux nœuds :

* 2 × STM32L476RG ;
* 2 × HC-12 ;
* 2 × BMP280 ;
* câbles Dupont / alimentation ;
* ordinateurs permettant de communiquer avec les cartes.

Pour reproduire une expérience multi-sauts, au moins **3 nœuds** sont nécessaires.

---

### Logiciels

L'environnement initial du projet utilise notamment :

* **STM32CubeMX** pour la configuration des périphériques et horloges ;
* **STM32CubeProgrammer** pour programmer les cartes ;
* **Visual Studio Code** pour le développement ;
* **PuTTY** pour l'affichage des informations de debug via liaison série.

STM32CubeIDE peut également être utilisé comme environnement de développement alternatif, mais il ne correspond pas à l'environnement initial utilisé pour le développement du projet.

---

## Configuration matérielle

Lorsque le shield utilisé pour le prototype n'est pas disponible, les modules peuvent être câblés directement sur la carte.


Le BMP280 utilise l'interface **I²C**.

Le HC-12 utilise une liaison série avec le STM32.

Une liaison série supplémentaire est utilisée pour afficher les informations de debug sur le terminal.

---

## Compilation et programmation

Le projet contient plusieurs configurations correspondant notamment au nœud standard et au nœud central.

### 1. Générer le projet

Configurer le microcontrôleur et ses périphériques avec STM32CubeMX puis compiler le projet avec l'environnement de développement utilisé.

Le fichier `.elf` généré est ensuite utilisé pour programmer la carte.

### 2. Programmer une carte

Avec STM32CubeProgrammer :

```text
Open file
    ↓
build/debug/interruptions.elf
    ↓
Download
```

### 3. Programmer le nœud central

Pour le nœud central, utiliser la version du code correspondant au fichier `noeud_central.c` selon l'organisation actuelle du projet.

### 4. Configurer le terminal

Ouvrir PuTTY ou un terminal série équivalent.

Utiliser :

```text
Baud rate : 9600
```

Sélectionner le port COM correspondant à chaque carte dans le gestionnaire de périphériques.

---

## Lancer une expérience à deux nœuds

1. Programmer la première carte avec le code du nœud.
2. Programmer la seconde carte avec le code du nœud central.
3. Ouvrir les terminaux série correspondant aux deux cartes.
4. Régler le débit à **9600 bauds**.
5. Réinitialiser les cartes.
6. Démarrer le nœud central en dernier afin qu'il initie la procédure d'attribution du gradient.
7. Observer les échanges et les métriques dans les terminaux.

Les informations affichées permettent notamment de suivre les transmissions, ACK, erreurs et temps d'échange.

---

## Configuration de plusieurs nœuds

Pour ajouter des nœuds au réseau, programmer chaque STM32 avec le code correspondant puis attribuer une adresse différente à chaque carte.

La constante :

```c
uint8_t node_addr
```

permet d'identifier individuellement les nœuds.

Une configuration à trois nœuds permet notamment d'expérimenter le fonctionnement multi-sauts et le routage par gradient.

---

## Tests et résultats

Une partie importante du projet a été consacrée à la caractérisation expérimentale du réseau.

Les expériences ont été réalisées en essayant de maintenir les conditions expérimentales aussi constantes que possible et en faisant varier un paramètre à la fois.

Les principales métriques étudiées sont :

* **Packet Error Rate (PER)** ;
* **Bit Error Rate (BER)** ;
* taux d'acquisition des paquets ;
* **Round Trip Time (RTT)** ;
* débit utile ;
* consommation énergétique ;
* puissance d'émission ;
* portée effective.

### Fiabilité de la communication

Les essais montrent une dégradation du taux d'acquisition avec l'augmentation de la distance.

L'ajout du mécanisme de récupération basé sur le Hamming améliore le comportement du système dans les conditions expérimentées.

Dans les essais présentés dans le rapport, le BER observé avant l'ajout du mécanisme de correction pouvait atteindre des valeurs importantes dans les conditions les plus défavorables. Après l'ajout du mécanisme de récupération, aucune erreur n'a été détectée dans la batterie de tests considérée. Cela ne permet toutefois pas d'affirmer que le BER est mathématiquement nul.

### Hamming et consommation

L'ajout du Hamming augmente la quantité de données transmises : les données sont encodées par groupes de 4 bits et la trame passe de **6 octets de données à 12 octets transmis**.

Malgré cette augmentation, les mesures présentées dans le rapport montrent un intérêt énergétique dans les conditions testées, notamment grâce à la réduction du nombre de retransmissions.

Une mesure expérimentale donne notamment :

```text
Retransmission : ~15 500 µJ
Hamming        : ~6 500 µJ
```

### Débit multi-sauts

L'ajout de nœuds intermédiaires introduit une latence supplémentaire.

Une optimisation importante a consisté à retirer les appels UART bloquants utilisés uniquement pour le debug dans le chemin critique de transmission.

Le délai de traitement mesuré est alors passé à environ :

```text
361 µs
```

soit une amélioration d'environ **465×** par rapport à la configuration initiale.

### Consommation

Les différents modes basse consommation du STM32 ont été comparés.

Le mode **STOP2** a été retenu pour le fonctionnement périodique du nœud en raison de sa faible consommation.

Le projet met également en évidence une limitation importante du HC-12 : sa consommation en attente constitue un poste énergétique difficilement compressible avec le matériel utilisé.

Une étude théorique avec une Wake-Up Radio montre qu'une architecture de ce type pourrait fortement augmenter l'autonomie. Pour une batterie théorique de 3000 mAh, le modèle présenté dans le rapport estime une autonomie passant d'environ **35 jours à 250 jours** dans le scénario considéré.

La Wake-Up Radio n'est toutefois **pas intégrée au prototype expérimental**.

---

## Compromis puissance / portée / énergie

Les différents niveaux de puissance du HC-12 permettent d'étudier directement le compromis entre consommation et qualité du lien radio.

La puissance d'émission peut être configurée de :

```text
-1 dBm → 20 dBm
```

selon le niveau sélectionné.

Les essais montrent qu'une puissance d'émission plus élevée améliore généralement la fiabilité du lien mais augmente la consommation.

Le choix du mode de transmission doit donc prendre en compte simultanément :

* la distance entre les nœuds ;
* la qualité du canal ;
* la fiabilité recherchée ;
* le débit ;
* le temps d'activité ;
* l'énergie disponible.

---

## Modélisation

Le projet comprend également une partie de modélisation des performances du réseau.

Plusieurs niveaux de granularité ont été étudiés pour représenter notamment :

* le taux d'erreur par paquet ;
* le taux d'erreur binaire ;
* l'influence de la distance ;
* la puissance reçue ;
* les variations du canal ;
* le débit en fonction du nombre de sauts ;
* la consommation énergétique.

L'objectif est d'identifier le modèle le plus pertinent selon le niveau de précision recherché et la partie du système étudiée.

---

## État du projet

| Fonctionnalité                                   | État                                   
| ------------------------------------------------ | -------------------------------------- 
| Communication HC-12                              | ✅ Testée                               
| Communication BMP280 / I²C                       | ✅ Testée                               
| ACK et retransmission                            | ✅ Testés                               
| Mesure RTT                                       | ✅ Testée                               
| Mesure PER / BER                                 | ✅ Testée                               
| Communication multi-nœuds                        | ✅ Testée                               
| Routage par gradient                             | ✅ Implémenté et testé                  
| TDMA                                             | ✅ Implémenté et testé                        
| Hamming (8,4)                                    | ✅ Implémenté et testé                 
| STOP2                                            | ✅ Implémenté et testé                  
| Réveil RTC                                       | ✅ Implémenté et testé                          
| Profilage énergétique des voisins                | Implémenté, validation limitée      
| Adaptation dynamique du gradient selon l'énergie | Fonctionnalité expérimentale        
| Wake-Up Radio                                    | Étudiée théoriquement, non intégrée 

---

## Limites

Le prototype présente plusieurs limitations liées au matériel et aux choix expérimentaux.

Le HC-12 constitue notamment une part importante de la consommation du système lorsqu'il reste en attente d'une communication. Il n'est pas possible de modifier directement l'architecture interne de sa chaîne radio.

Les mécanismes de basse consommation du module imposent également des compromis importants entre vitesse de transmission, sensibilité et portée.

Enfin, certaines fonctions développées en fin de projet, notamment celles liées à l'adaptation énergétique dynamique, nécessitent davantage de validation expérimentale avant de pouvoir être considérées comme définitives.

---

## Perspectives

Plusieurs améliorations peuvent être envisagées :

* comparer plusieurs algorithmes de routage pour WSN ;
* comparer différentes combinaisons de mécanismes de récupération ;
* approfondir le profilage énergétique des nœuds ;
* améliorer l'adaptation dynamique du réseau ;
* étendre les expérimentations à un nombre plus important de nœuds ;
* améliorer la synchronisation et le protocole TDMA ;
* intégrer une véritable **Wake-Up Radio** ;
* remplacer le module radio actuel par une solution permettant un contrôle plus fin de la consommation ;
* poursuivre la comparaison entre modèles théoriques et mesures expérimentales.

---

## Contexte

Projet réalisé dans le cadre d'un stage assistant ingénieur :

**Optimisation de la consommation d'un réseau de capteurs sans fil**

**Laboratoire :** Laboratoire d'Électronique, Antennes et Télécommunications (LEAT)
**Équipe :** EDGE — Edge Computing et Systèmes Numériques
**Structure de soutien :** Digital Systems for Humans (DS4H)
**Établissement :** Polytech Nice Sophia / Université Côte d'Azur
**Année :** 2026

---

## Références

Les principales références scientifiques et documentaires utilisées pour le projet sont disponibles dans le rapport de stage associé.

Elles comprennent notamment :

* documentation STM32L476xx de STMicroelectronics ;
* documentation du module HC-12 ;
* documentation du BMP280 ;
* travaux scientifiques sur les WSN basse consommation ;
* travaux sur les protocoles de récupération de paquets ;
* travaux sur les mécanismes de duty cycling et Wake-Up Radio.
