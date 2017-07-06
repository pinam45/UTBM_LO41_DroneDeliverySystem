# Simulation de système de livraison par drone #

Ce programme à été développé dans le cadre de l'UV LO41 «Système d'Exploitation : Principes et Communication».
Il s'agit d'une simulation de système de livraison par drone.

Un vaisseau mère transporte des drones et des colis et une fois arrivé au point de livraison, le vaisseau mère assigne des colis aux drones qui se chargent de les livrer.
La simulation suppose que le vaisseau mère est déjà placé au bon endroit.

## Comment compiler le programme ##

Pour compiler le programme il suffit d'exécuter la commande 'make' à la racine du projet.

## Comment utiliser le programme ##

Exécuter la commande './build/bin/UTBM\_LO41\_DroneDeliverySystem.elf' pour lancer le programme.
Le programme peut également prendre en paramètre des arguments, ces arguments sont les fichiers de configuration de la simulation.
On peut ainsi changer les colis, les drones et les clients.

On exécutera alors la commande suivante './build/bin/UTBM\_LO41\_DroneDeliverySystem.elf PACKAGES\_FILE CLIENT\_FILE DRONE\_FILE'

## Comment éditer les fichiers de configuration ##

### Fichier package ###

Les colis sont représentés comme il suit:

Identifieur unique aux colis, priorité, poid, Identifieur du client, Nombre d'essais maximum

### Fichier drone ###

Les drones sont représentés comme il suit:

Identifieur unique aux drones, charge maximum, autonomie, temps de recharge

### Fichier client ###

Les clients sont représentés comme il suit:

Identifieur unique aux clients, distance au vaisseau mère
