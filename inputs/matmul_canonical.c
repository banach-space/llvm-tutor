void matmul(double A[800][800], double B[800][800], double C[800][800]) {
  for (int i = 0; i < 800; i++) {
    for (int j = 0; j < 800; j++) {
      double sum = 0.0;
      for (int k = 0; k < 800; k++) {
        sum += A[i][k] * B[k][j];
      }
      C[i][j] = sum;
    }
  }
}

