# OpenCL


In this study I implemented Powershift method using OpenCL 
eigvec1 = ones(12,1);

 for (i=0; i<10; i++)        
    eigvec1 = A * eigvec1;    //this will be done in parallel        
    eigvec1 = eigvec1/norm(eigvec1); //this will be done in parallel
    eigval1 = 〖eigvec1〗^T * A * eigvec1; //this will be done in parallel
  end

  eigvec2 = ones(12,1);

B = A - eigval1 * (eigvec1 * eigvec1^T);  
// eigval1 and eigvec1 are the largest eigenvalue and its  eigenvector of A
// this will be done in parallel
    
  for (i=0;i<10;i++)       
    eigvec2 = B * eigvec2;    //this will be done in parallel        
    eigvec2 = eigvec2/norm(eigvec2); //this will be done in parallel
    eigval2 = x^T * B * x;
  end
