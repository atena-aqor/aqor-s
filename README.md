# Architecture logicielle du projet AQOR
## Présentation du projet


### Architecture matérielle

 Le logiciel du projet AQOR a été développé en 3 parties distinctes qui reflètent l’architecture matérielle montrée ici:
 
![Architecture](img/Architecture.png)

### Architecture logicielle

Elle comprend:
- Un programme implanté dans le boîtier Emetteur qui fonctionne sur batterie
- Un programme implanté dans le boîtier Passerelle qui est connecté en WiFi à la box internet de l’hébergeur 
- Un logiciel Open Source de collecte et de diffusion des données implanté sur un micro-ordinateur Raspberry Pi5 fonctionnant sous linux UBUNTU. 

Les codes sources des programmes d'**émission** et de **passerelle** sont décrits dans les répertoires correspondants. Les fichiers Readme.md de chaque répertoire permettent de connaître le rôle de chaque fichier 

### Compilation des fichiers arduino 

Les fichiers proposés sont compilables sur la version de l'IDE Arduino 1.8.19 dont il faut choisir les préférences indiquées dans la figure suiivante 

![Préférences Arduino 1.8.19](img/prefrences_arduino_ide.jpg)

le lien exacte est [https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/1.0.0/package_heltec_esp32_index.json](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/1.0.0/package_heltec_esp32_index.json)
