//AB's first complete C function!! Yay!!!
/*
The purpose of this is basically to impose constraints given by a constraint file
IN the constraint file, the header should have # in front
And for each row, the first two columns give residue pairs that are constrained by a disulfide link
These should be given in zero indexing
The last row gives relative weight for that constraint

Recommended reading:
https://craftofcoding.wordpress.com/2015/12/07/memory-in-c-the-stack-the-heap-and-static/




*/


int Argmin( float distances[]);
void Read_constraints();
float Compute_constraint_energy();



int Argmin( float distances[]){
	int argmin=0;
	int i;
	for (i = 0; i < N_constraints; i++){
		if (distances[i]<distances[argmin]){
			argmin=i;
		}
	}
	return argmin;
}


void Read_constraints () {
	//This code is written by AB whose a NEWBIE to C so lots of comments!!! ;P ;P ;P
	//https://www.cs.swarthmore.edu/~newhall/unixhelp/C_arrays.html
	//Am using Method 2 under dynamically allocated 2D arrays (but changed Malloc to Calloc based on some advantages I read online)

    FILE *fp;
    char str[1000];
    int aa, bb;
    float cc;
    
    int iii=0;
    
    
     
    //We already declare the following in backbone.h 
    //int Max_N_constraints = 1000;
    //int N_constraints;
    //int **constraint_array; //We will declare a pointer to an array of Max_N_constraints pointers to ints...the ** indicates pointer to a pointer
    
    //float *constraint_weights; //A pointer to an array
    
    constraint_array = (int **)calloc(Max_N_constraints, sizeof(int *)); //Soo...we are allocating an array of pointers ( Max_N_constraints of them)
    			//The calloc function is saying that we want to allocate memory for an array of Max_N_constraints integer pointers
    			//The (int **) at the beginning is saying that malloc should return a pointer to an array of pointers to ints (the memory for this array has been allocated)
    
    constraint_weights = (float*)calloc(Max_N_constraints,sizeof(float)); //Return a pointer to an array of floats
    
    //char* filename = "Sample_constraint.txt";
 
    fp = fopen(constraint_file, "r");
    if (fp == NULL){
        fprintf(STATUS, "Could not open file %s \n",constraint_file);
        exit(1);
    }
    else{
    fprintf(STATUS, "Successfully opened constraint file %s \n",constraint_file);
    fflush(STATUS);
    }
    while (fgets(str, 1000, fp) != NULL)
        //fprintf(STATUS, "%s \n", str);
        //fflush(STATUS);
        if (str[0] != '#'){
        	//fprintf(STATUS, "%s", str);
        	//fflush(STATUS);
        	sscanf(str, "%d %d %f", &aa, &bb, &cc );
        	//fprintf(STATUS, "Did sscanf \n");
        	//fflush(STATUS);
        	//fprintf(STATUS, "The a value is %d while the iii value is %i \n", aa, iii); 
        	//fflush(STATUS);
        	
        	constraint_array[iii] = (int *)calloc(2,sizeof(int)); //This is saying that the ith element of constrain_array is a pointer to memory for 2 integers
        	
        	constraint_array[iii][0] = aa;
        	constraint_array[iii][1] = bb;
        	constraint_weights[iii] = cc;
        	iii++;
        //fprintf(STATUS, "Hi this is line %i \n", iii);
        //fflush(STATUS);
        }

    fclose(fp);
    N_constraints = iii;
    fprintf(STATUS, "We have %i constraints. The 3rd constraint is %i, %i with weight %f \n", N_constraints, constraint_array[2][0], constraint_array[2][1], constraint_weights[2] );
    fflush(STATUS);
    //free(constraint_array); //In casse we didn't actually have Max_N_constraints, this frees unused memory--actually this didn't work as expected...it actually seemed to totally delete the variable
    //free(constraint_weights);
    
    
	distances = (float*)calloc(N_constraints,sizeof(float));  //Will contain distance between i and j
	disulfide_pairs_attempt = (int*)calloc(N_constraints,sizeof(int));  //Note that when you allocate an array with calloc, the default value for the elements is zero. This is in contrast to malloc
    
    return;
}





float Compute_constraint_energy (const struct residue *residues, const struct atom *atoms){	
	float constraint_energy=0;
	int i, res1, res2, atomi, atomj;
	float weight, xi, yi, zi, xj, yj, zj, dist;
	float sum_of_weights = 0;
	
	
	
	
	int min_ind;
	float min_dis;
	
	int I;
	int J;
	int zzz;

	
	
	//disulfides = (int*)calloc(N_cysteines,sizeof(int)); //Is residue i participating in disulfide?
	
	int disulfides[1000] =  { 0 }; //Tels you whether each residue is participating in a disulfide
	
	mean_constraint_distance = 0;
	
	for (i = 0; i < N_constraints; i++){
		res1 = constraint_array[i][0];
		res2 = constraint_array[i][1];
		weight = constraint_weights[i];
		
		//To do, check that neither i nor j are already forming a disulfide bond before proceeding
		
		atomi = residues[res1].CA;
      	xi = atoms[atomi].xyz.x;
      	yi = atoms[atomi].xyz.y;
      	zi = atoms[atomi].xyz.z;
      	
		atomj = residues[res2].CA;
      	xj = atoms[atomj].xyz.x;
      	yj = atoms[atomj].xyz.y;
      	zj = atoms[atomj].xyz.z;
      	

      	dist = sqrt(((xi-xj)*(xi-xj) +
		  (yi-yj)*(yi-yj) +
		  (zi-zj)*(zi-zj)));
		
		distances[i] = dist;
		disulfide_pairs_attempt[i]=0;  //for now, reset the value of disulfide..we will figure out later if a disulfide is present
	}
	
	min_ind=Argmin(distances);
	min_dis=distances[min_ind];
	
	//if (mcstep==0){
	//fprintf(STATUS, "Let's check on our variable distances at its initialization \n");
	//for (zzz=0; zzz<N_constraints; zzz++){
	//	fprintf(STATUS, "%i and %i: %f \n",  constraint_array[zzz][0],constraint_array[zzz][1], distances[zzz]);
	//}
	//}
	
	
	while (min_dis<=6){
		I=constraint_array[min_ind][0];
		J=constraint_array[min_ind][1];
		//if (mcstep==0){
		//	fprintf(STATUS, "The current min_dist is %f and it involves residues %i and %i \n", min_dis, I, J );
		//}
		if (disulfides[I]==0 && disulfides[J]==0){ //neither residue is already engaged in disulfide
			disulfides[I]=1; //well, now they are!
			disulfides[J]=1;
			disulfide_pairs_attempt[min_ind]=1;
			weight=constraint_weights[min_ind];
			constraint_energy=constraint_energy+weight*k_constraint*((min_dis - 5)*(min_dis -5)  - (6 - 5)*(6 -5) ); //Creates a Morse-like potential. WHen distance=6, no energy is contributed, for distance between 4 and 6, negative energy contributed, for distance less than 4, positive energy contributed!
			//if (mcstep==0){
			//	fprintf(STATUS, "These residues are now participating in a disulfide! We have \n (min_dist - 5) = %f \n (min_dist - 5)*(min_dist-5) = %f \n  (min_dist - 5)*(min_dist -5)  - (6 - 5)*(6 -5)  = %f \n The energy is %f \n", min_dist-5, (min_dist - 5)*(min_dist -5), (min_dist - 5)*(min_dist -5)  - (6 - 5)*(6 -5) , constraint_energy );
			//}
		}
		distances[min_ind]=INFINITY;
		min_ind=Argmin(distances);
		min_dis=distances[min_ind];
	}
	
	//if (mcstep==0){
	//fprintf(STATUS, "Let's check on our variable disulfide_pairs_attempt");
	//for (zzz=0; zzz<N_constraints; zzz++){
    //  if (disulfide_pairs_attempt[zzz]==1){
    //  	fprintf(STATUS, "%i and %i \n", constraint_array[zzz][0],constraint_array[zzz][1]);
    //  } 
  	//  }
	//}
	
	
	return constraint_energy;
}



///////////////////////////////////////////////

float Compute_constraint_energy_old (const struct residue *residues, const struct atom *atoms){
	
	float constraint_energy=0;
	int i, res1, res2, atomi, atomj;
	float weight, xi, yi, zi, xj, yj, zj, dist;
	float sum_of_weights = 0;
	
	mean_constraint_distance = 0;
	
	for (i = 0; i < N_constraints; i++){
		res1 = constraint_array[i][0];
		res2 = constraint_array[i][1];
		weight = constraint_weights[i];
		
		//To do, check that neither i nor j are already forming a disulfide bond before proceeding
		
		atomi = residues[res1].CA;
      	xi = atoms[atomi].xyz.x;
      	yi = atoms[atomi].xyz.y;
      	zi = atoms[atomi].xyz.z;
      	
		atomj = residues[res2].CA;
      	xj = atoms[atomj].xyz.x;
      	yj = atoms[atomj].xyz.y;
      	zj = atoms[atomj].xyz.z;
      	

      	dist = sqrt(((xi-xj)*(xi-xj) +
		  (yi-yj)*(yi-yj) +
		  (zi-zj)*(zi-zj)));
		
		
		constraint_energy = constraint_energy+ weight*k_constraint*(dist - 5)*(dist -5);
		
		//Add here: if this dist is between 4 and 6, add residues i and j to the list of residues
		//that are already part of a disulfide bond
		sum_of_weights = sum_of_weights + weight;
		mean_constraint_distance = mean_constraint_distance + weight*dist;
	
	//if (mcstep % MC_PRINT_STEPS == 0) {
	//fprintf(STATUS, "%Distance  is %5.3f \n", dist );
	//fflush(STATUS);	
	//}
	
	
	}
	mean_constraint_distance = mean_constraint_distance/sum_of_weights;

	//	if (mcstep % MC_PRINT_STEPS == 0) {
	//fprintf(STATUS, "Constraint energy is %5.3f \n", constraint_energy);
	//fflush(STATUS);	
	//}

	return constraint_energy;
}

