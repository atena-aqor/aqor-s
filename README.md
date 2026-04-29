#Architecture logicielle du projet AQOR
# 


### Architecture matérielle Le logiciel du projet AQOR a été développé en 3 parties distinctes qui reflètent l’architecture matérielle montrée ici
![Architecture](img/Architecture.png)
Il comprend
- Un programme implanté dans le boîtier Emetteur qui fonctionne sur batterie
- Un programme implanté dans le boîtier Passerelle qui est connecté en WiFi à la box internet de l’hébergeur 
- Un logiciel Open Source de collecte et de diffusion des données implanté sur un micro-ordinateur Raspberry Pi5 fonctionnant sous linux UBUNTU. 
Les codes sources des programmes d'émission et de passerelle sont décrits dans les répertoires correspondants et 
