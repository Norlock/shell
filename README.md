<style type="text/css">
	#img-main-page {
		display: block;
		margin-left: auto;
		margin-right: auto;
		width: 40%;
	}

	#title {
		color: #222;
		display:block;
		width: 100%;
		text-align: center;
		font-size: 36px;
	}

	#sub-title {
		color: #333;
		display:block;
		width: 100%;
		text-align: center;
		font-size: 26px;
	}
</style>

<span id="title">Shell</span>
<span id="sub-title">Compilers en Operating Systems</span>

<img id="img-main-page" src="http://icons.iconarchive.com/icons/alecive/flatwoken/512/Apps-Terminal-Pc-104-icon.png">

<br>

Gemaakt door: Joris Willems<br>
Student nummer: 349672<br>
Datum: 3-6-2018<br>

---

# Inhoudsopgave
1. [Inleiding](#Inleiding)
2. [Uitvoeren van commando's](#commando)
3. [Omleiden van I/O](#redirects)
4. [Aanleggen van pipes](#pipes)
5. [Asynchroon aanroepen van processen](#async)

# Inleiding <a name="Inleiding"></a>

Een shell is een command-line interpreteerder. Via commando's kunnen programma's worden uitgevoerd. Een voorbeeld is het 
programma ls(**l**ist **d**irectory **c**ontents), die een weergave geeft van de bestanden en folders binnen het
opgegeven pad. Behalve dat een shell programma's kan starten, kan een shell ook de invoer en uitvoer omleiden. Zo kan
het ls programma bijvoorbeeld op verschillende manieren worden omgeleid:

```sh
ls > test.txt # Schrijft de uitvoer van het programma ls naar bestand test.txt (Redirect output)
cat < test.txt # Gebruikt het bestand test.txt als invoer (Redirect input)
ls >> test.txt # Voegt de uitvoer van het programma ls toe aan het einde van bestand test.txt (Redirect append)
```

File descriptors(fd) zijn abstracte indicatoren voor bronnen zoals bestanden of invoer/uitvoer. Deze file descriptors
zijn onderdeel van de POSIX API. POSIX is een familie van standaarden die ervoor moeten zorgen dat er compatibiteit is
tussen besturingssystemen. Deze standaarden wordt gebruikt door alle UNIX OS varianten (Linux / Apple).

Deze bronnen (invoer/uitvoer/append/bestanden...) worden uiteindelijk allemaal een nummer in de file descriptors table.
Elk proces binnen een shell heeft drie standaard file descriptors:

| Integer value | Name | <unistd.h> symbolic constant[1] | <stdio.h> file stream[2] |
|:-------------:|------|---------------------------------|--------------------------|
| 0 | Standard input | STDIN_FILENO | stdin |
| 1 | Standard output | STDOUT_FILENO | stdout |
| 2 | Standard error | STDERR_FILENO | stderr |

De file descriptors voor bestanden (e.g. test.txt als invoer) zijn vanaf integer 3 en hoger.

# Uitvoeren van commando's <a name="commando"></a>
De code voor het starten van programma's met de meegegeven parameters staat in het bestand SimpleCommand.cpp. Waar eerst
naar gekeken wordt is of het commando gelijk staat aan cd (change directory).

```c++
if(command == "cd") {
	const char *argumentPath = arguments[0].c_str();
	int ret;
	ret = chdir(argumentPath);
	if(ret == -1) 
		std::cerr << "Directory does not exist" << std::endl;

	return;
}
```

Op het moment dat de ingevoerde commando gelijkstaat aan cd wordt het meegegeven pad aan het chdir programma meegegeven.
Als de directory niet bestaat zal er een foutmelding worden gegeven. In UNIX is het een standaard dat programma's die
falen een -1 terug geven. Wanneer het opgegeven commando niet gelijk is aan cd, wordt er gekeken of er redirects opgegeven zijn.
Als er geen redirects aanwezig zijn wordt het commando uitgevoerd:

```c++ <a name="commando-code"></a>
if ((pid = fork()) == -1)
	perror("fork() error");
else if (pid == 0) {
	execvp(command.c_str(), parmList);
	std::cerr << "Return not expected. Process failed." << std::endl;
}
waitpid(pid, NULL, 0); // Don't write to stdout before child process is finished.
```

Het commando fork maakt kopie van zijn eigen proces aan. Het process dat een 0 terug geeft is het kind proces. Het proces dat een
getal boven de 0 terug geeft is het kind process van de ouder. Waitpid is de methode die wacht totdat de return waarde 0
(kind) bij deze methode is aangekomen zal de parent door deze methode vallen. execvp is een methode die gebruikt wordt
om een programma te starten. De execvp methode zal kijken naar binaries binnen de /usr/bin directory. De parameters voor
de methode zijn als volgt:

```c++
int execvp(const char *file, char *const argv[]);
```

Het eerste argument is het commando / binary file, zoals ls of pwd. Het tweede argument is een array waarin het eerste
argument de commando ofwel de binary file is, vervolgens wordt de array gevuld alle argumenten (-l -a, etc) en uiteindelijk afgesloten met een NULL waarden. 

```c++
parmList[0] = (char*)command.c_str();

if(argumentsInputSize > 0) {
	for(std::vector<int>::size_type i = 0; i != argumentsInputSize; i++) {
		parmList[i+1] = strdup((char*)arguments[i].c_str());
	}
}
parmList[argumentsArraySize - 1] = NULL; 
```

# Omleiden van I/O <a name="redirects"></a>

Wanneer er omleidingen zijn meegegeven wordt er gekeken of deze omleiding een invoer,
uitvoer of toevoeging betreft.

```c++
if(redirects.size() > 0) {
	for(std::vector<int>::size_type i = 0; i != redirects.size(); i++) {
		int redirectType = redirects[i].getType();
		std::string filePath = redirects[i].getNewFile();

		if(redirectType == IORedirect::OUTPUT) {
			const int fileDescriptor = open(filePath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);

			if(fileDescriptor != -1) { // Does File exist?
				execRedirect(IORedirect::STDOUT, fileDescriptor, parmList);
			}
			else {
				  std::cerr << "Error creating file." << std::endl;
			}
		}
		else if (redirectType == IORedirect::INPUT) {
			const int fileDescriptor = open(filePath.c_str(), O_RDONLY);

			if(fileDescriptor != -1) {
				execRedirect(IORedirect::STDIN, fileDescriptor, parmList);
			}
			else {
				std::cerr << "File not found" << std::endl;
			}
		}
		else if (redirectType == IORedirect::APPEND) {
			const int fileDescriptor = open(filePath.c_str(), O_WRONLY | O_APPEND);

			if(fileDescriptor != -1) { // Does File exist?
				execRedirect(IORedirect::STDOUT, fileDescriptor, parmList);
			}
			else {
				  std::cerr << "File not found." << std::endl;
			}
		}
	}
}
```

Wanneer de redirect naar een output geschreven wordt (e.g. ls > test.txt), zal het opgegeven bestand geopend worden met de
open() methode. Aan deze methode kunnen een aantal eigenschappen worden meegegeven, zoals bijvoorbeeld O_TRUNC dat in
het voorbeeld hierboven te zien is. O_TRUNC geeft aan dat een nieuw bestand aangemaakt moet worden mits deze nog niet
bestaat. Verder kunnen er ook schrijf rechten worden meegegeven als het bestand nog niet zou bestaan. De execRedirect()
is een methode die het aangeroepen programma start en de uitvoer of invoer omleid.  

```c++
void execRedirect(const int fdFrom, const int fdTo, char* parmList[]);
```

Deze methode heeft twee file descriptors nodig: van en naar. Bij ls > test.txt wordt de fd van output omgeleid naar de
fd van test.txt. 

# Aanleggen van pipes <a name="pipes"></a>

Met een UNIX shell is het ook mogelijk om twee processen met elkaar te verbinden. Dit kan door middel van de file
descriptors met elkaar te verbinden. De shell weet dat met de "|" karakter een pipe moet worden aangelegd.  

```sh
ls | grep s 
```

Als het ls commando in de project folder wordt uitgevoerd zal de uitvoer als volgt zijn:

```
CMakeCache.txt  CMakeFiles  CMakeLists.txt  Makefile  README.md  ShellGrammar.g4  cmake_install.cmake  gen  runtime  shell  src  tags  test.txt
```

De uitvoer van dit commando kan vervolgens worden gegeven aan het volgende commando "grep s" en de uitvoer zal dan als
volgt zijn:

```
CMakeFiles
CMakeLists.txt
cmake_install.cmake
shell
src
tags
test.txt
```

Van de oorspronkelijk uitvoer blijven alleen de resultaten die een **s** bevatten over.

In de klasse Pipeline.cpp worden deze verbindingen gemaakt, eerst wordt er gekeken of er meerdere commando's
(processen/programma's) zijn aangeroepen. Als er meerdere commando's zijn opgegeven binnen een pipeline betekend dat
deze gekoppeld moeten worden, anders zal het commando direct worden uitgevoerd. 

```c++
int pipeFdsOld[2], pipeFdsNew[2];

while (commandIndex < commandsSize) {
	bool hasNextCommand = commandIndex < pipeSize;
	bool hasPreviousCommand = 0 < commandIndex;

	if(hasNextCommand) {
		if(pipe(pipeFdsNew) < 0) {
			perror("Pipe error");
			exit(EXIT_FAILURE);
		}
	}
	...
}
```

Deze while loop wordt gebruikt om alle pipes te koppelen. Om te bepalen welke pipes gekoppeld moet worden, wordt er
eerst gekeken of er een volgend commando aanwezig is. Wanneer een volgend commando aanwezig is zal er een
pipe worden gemaakt. De pipe methode heeft als parameter een integer array nodig die twee elementen bevat. De pipe methode zal deze
array vullen met een file descriptor voor lezen en een file descriptor voor schrijven. 

```
pid_t pid = fork();
if (pid == 0) {

	if(hasPreviousCommand) {
		if(dup2(pipeFdsOld[0], STDIN_FILENO) < 0) {
			perror("Pipe error");
			exit(EXIT_FAILURE);
		}
		close(pipeFdsOld[0]);
		close(pipeFdsOld[1]);
	}

	if(hasNextCommand) {
		close(pipeFdsNew[0]);
		if(dup2(pipeFdsNew[1], STDOUT_FILENO) < 0) {
			perror("Pipe error");
			exit(EXIT_FAILURE);
		}
		close(pipeFdsNew[1]);
	}

	commands[commandIndex]->execute();
	exit(EXIT_FAILURE);
} ... 
```

Nu deze pipes zijn aangelegd betekend nog niet dat de omleidingen zijn aangelegd. Dit zal in het kind proces gebeuren.
Het kind proces zal wanneer er een vorige commando is deze file descriptors omleiden (van standaard invoer naar
pipeFdsOld[0]). De file descriptors die omgeleid worden zijn door de pipe methode van het vorige voorbeeld gevuld. 

Als deze omleiding zijn aangelegd wordt het commando uitgevoerd. In het geval dat het volgende commando is opgegeven:

```sh
ls | grep s | grep c
```

Is nu "ls" uitgevoerd door het kindproces. Pas in de volgende iteratie wordt het volgende commando uitgevoerd. 

```c++
} else if (pid < 0) {
	perror("Pipe error 1");
	exit(EXIT_FAILURE);
} else {
	if(hasPreviousCommand) {
		close(pipeFdsOld[0]);
		close(pipeFdsOld[1]);
	}

	if(hasNextCommand) {
		pipeFdsOld[0] = pipeFdsNew[0];
		pipeFdsOld[1] = pipeFdsNew[1];
	}
}

commandIndex++;
```

De verbinding is gemaakt voor dit proces maar wordt voor het volgende kindproces al klaar gezet. Het volgende kind
process "grep s" kan de old file descriptors gebruiken voor zijn invoer.

```c++
// Parent closes all of its copies
close(pipeFdsOld[0]);
close(pipeFdsOld[1]);

for(int i = 0; i < pipeSize + 1; i++)
	wait(&status);
```

Wanneer alle commands zijn geweest. sluit de parent zijn file descriptor kopieen. Vervolgens wacht de shell op alle
childs. 

# Asynchroon aanroepen van processen <a name="async"></a>

In shells is het mogelijk om processen op de achtergrond te laten draaien, zodat dit programma de shell niet blokeerd.
Deze processen draaien asynchroon op de achtergrond, een unix shell weet dat een proces asynchroon is wanneer een
commando eindigt met een "&" teken. Een voorbeeld hiervan is:

```sh
firefox &
```

Firefox zal nu op de achtergrond gestart worden.

```c++
for( Pipeline *p : pipelines ) {
	if(p->isAsync()) {
		pid_t pid = fork();
		if(pid == 0) { // Child process
			std::cout << "Runs background process with id: " << getpid() << std::endl;
			p->execute();
			exit(EXIT_FAILURE);
		} else if (pid < 0) {
			std::cerr << "Command failed, can't create child process" << std::endl;
		}
	} else {
		p->execute();
	}
}
```

Wanneer het proces asynchroon is, wordt het gestart door een kindproces. De shell zal aangeven welk process id dit
programma heeft. Om dit process te stoppen kan doormiddel van het commando kill met het nummer van process. 

```sh 
kill 3469
```


