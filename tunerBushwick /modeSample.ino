int mode(int *x, int n) {
  int i = 0;
  int count = 0;
  int maxCount = 0;
  int mode = 0;
  int bimodal;
  int prevCount = 0;



  while (i < (n - 1)) {
    prevCount = count;
    count = 0;



    while (x[i] == x[i + 1]) {
      count++;
      i++;
    }
    
    if (count > prevCount & count > maxCount) {
      mode = x[i];
      maxCount = count;
      bimodal = 0;
    }

    if (count == 0) {
      i++;
    }

    if (count == maxCount) { //If the dataset has 2 or more modes.
      bimodal = 1;
    }

    if (mode == 0 || bimodal == 1) { //Return the median if there is no mode.
      mode = x[(n / 2)];
    }

    return mode;

  }

}
