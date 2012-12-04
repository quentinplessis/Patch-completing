						Complétion d'images par patchs - Readme

//Sous Windows 7 et Windows Vista

Les dossiers rendus et inputs doivent être dans le même dossier que l'exécutable.
Les dll suivantes doivent être avec l'exécutable : libopencv_core230.dll ; libopencv_highgui230.dll ; glut32.dll.
Ces dll ainsi que l'exécutable sont dans le dossier bin.

Exécution de l'application :

*sans arguments : applique l'algorithme de reconstruction pour l'image des colonnes 128x128 présente dans le dossier inputs.

*avec un argument quelconque : patch-completing.exe x
	Applique l'algorithme de reconstruction pour certaines images du dossier inputs pour différentes valeurs de la taille du patch et de tau.
	Les résultats sont stockés dans le dossier rendus.
*avec quatre arguments : patch-completing.exe entree masque patch tau
	Applique l'algorithme de reconstruction pour l'image entree avec le masque masque, en prenant patch pour taille de patch et tau pour valeur de tau.
	Les résultats sotn stockés dans le dossier rendus.
	