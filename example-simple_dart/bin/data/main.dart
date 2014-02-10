import 'dart:isolate';

class Sketch {
	Sketch() {
		print("created Sketch");
	}
 
	update() {
		print("update");
	}
 
	draw() {
		print("draw");
	}

	int getMeaningOfLife() {
		print("inGetMeaningOfLife");
		return 42;
	}

	List get_list() {
		List a = new List();
		a.add(0);
		a.add(2);
		a.add(4);
		return a;
	}
}
 
get_int() { 
	return 666; 
} 


main() 
{
	print("Hello, Darter!");
	print("Bingo!");
}