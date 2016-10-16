#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


//#define DEBUG

typedef struct
{
  int kind; // 0 = plane, 1 = sphere, 2 = camera
  double color[3];
  union {
    struct {
      double color[3];
      double position[3];
      double normal[3];
    } plane;
    struct {
      double color[3];
      double position[3];
      int radius;
    } sphere;
    struct {
      double width;
      double height;
    } camera;
  };
} Object;

typedef struct Pixel
  {
  unsigned char r, g, b;
  } Pixel;

Object** parseScene(char* input);
int nextChar(FILE* json);
int getC(FILE* json);
int checkNextChar(FILE* json, int val);
char* nextString(FILE* json);
char* checkNextString(FILE* json, char* value);
double* nextVector(FILE* json);
double nextNumber(FILE* json);
Pixel* raycast(Object** objects, int pxW, int pxH);
int planeIntersect(Object* object, double* rO, double* rD);
int imageWriter(Pixel* image, char* input, int pxW, int pxH);


int line = 1;

static inline double sqr(double v)
{
  return v*v;
}

static inline void normalize(double* v)
{
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}

int main (int c, char** argv)
{
  printf("hello\n");
  if(c != 5)
  {
    fprintf(stderr, "Error: Invalid number of arguments\n");
    exit(1);
  }
  Object** r = parseScene(argv[3]);
  int i = 0;
  while (r[i] != NULL)
  {
    int t = r[i]->kind;
    printf("%i\n", t);
    if(t == 0) //plane
    {
      for(int j = 0; j <3;j++)
      {
        printf("%lf ", r[i]->plane.color[j]);
      }
      printf("\n");
      for(int j = 0; j <3;j++)
      {
        printf("%lf ", r[i]->plane.position[j]);
      }
      printf("\n");
      for(int j = 0; j <3;j++)
      {
        printf("%lf ", r[i]->plane.normal[j]);
      }
      printf("\n");
    }
    else if(t == 1)
    {
      for(int j = 0; j <3;j++)
      {
        printf("%lf ", r[i]->sphere.color[j]);
      }
      printf("\n");
      for(int j = 0; j <3;j++)
      {
        printf("%lf ", r[i]->sphere.position[j]);
      }
      printf("\n");
      printf("%i\n", r[i]->sphere.radius);
    }
    else if(t == 2)
    {
      printf("%lf\n", r[i]->camera.width);
      printf("%lf\n", r[i]->camera.height);
    }
    i++;
  }
  int pxW = atoi(argv[1]);
  int pxH = atoi(argv[2]);
  Pixel* p = raycast(r,pxW,pxH);
  int q = imageWriter(p, argv[4],pxW, pxH);
  return q;
}

int imageWriter(Pixel* image, char* input, int pxW, int pxH)
{
  FILE* fw = fopen(input, "w"); // File Write as P3

  fprintf(fw, "P3\n");
  fprintf(fw, "%i ", pxH);
  fprintf(fw, "%i\n", pxW);
  fprintf(fw,"%i\n",255);

  int row, col;
  for (row = 0; row < pxH; row += 1) //itterate through image array
    {
    for (col = 0; col < pxW; col += 1)
      {
      fprintf(fw,"%i ", image[pxW*row + col].r);
      fprintf(fw, "%i ", image[pxW*row + col].g);
      fprintf(fw, "%i\n", image[pxW*row + col].b);
      }
    }
  fclose(fw);
}

int planeIntersect(Object* object, double* rO, double* rD)
{
  double* norm = object->plane.normal;
  double* pnt = object->plane.position;
  //a(x-x0) + b(y-y0)+c(z-z0) = 0
  //a((rOx + t*rDx - x0)) + b((rOy + t*rDy - y0)) + c((rOz + t*rDz - z0))
  //a*rOx + t*a*rDx - a*x0 + b*rOy + t*b*rDy - b*y0 + c*rOz + t*c*rDz - c*z0
  //t(a*rDx+ b*rDy+ c*rDz) + (a*rOx + b*roy + c*rOz - a*xO - b*yO - c*zO) = 0
  //t = -(a*rOx + b*roy + c*rOz - a*xO - b*yO - c*zO) / (a*rDx+ b*rDy+ c*rDz)
  //form: mt+b = 0
  double m = norm[0]*rD[0] + norm[1]*rD[1] + norm[2]*rD[2];
  double b = norm[0]*rO[0] + norm[1]*rO[1] + norm[2]*rO[2] - norm[0]*pnt[0] - norm[1]*pnt[1] - norm[2]*pnt[2];
  double t = (-1*b)/m;
  if(t >= 0)
  {
    return t;
  }
  else
  {
    return -1;
  }
}

int sphereIntersect(Object* object, double* rO, double* rD)
{
  double r = object->sphere.radius;
  double* pnt = object->sphere.position;
  // (x-h)^2 + (y-j)^2 + (z-k)^2 = r^2
  // (rOx - t*rDx-x0)^2 + (rOy - t*rDy-y0)^2 + (rOz - t*rDz-z0)^2 = r^2

  //rDx^2t^2 - 2rDxrOxt + 2rDxtx0+ rOx^2 - 2rOx x0 + x0^2
  // + rDy^2t^2 - 2rDyrOyt + 2rDyty0+ rOy^2 - 2rOyy0 + y0^2
  // + rDZ^2t^2 - 2rDZrOZt + 2rDZtZ0+ rOZ^2 - 2rOzz0 + z0^2
  // - r^2 = 0

  //quadratic function
  double a = sqr(rD[0] - rO[0]) + sqr(rD[1]- rO[1]) + sqr(rD[2]- rO[2]);

  double b = 2*((rD[0]* (rO[0] - pnt[0])) + (rD[1]* (rO[1]- pnt[1])) + (rD[2]*(rO[2]- pnt[2])));

  double c = sqr(rO[0]- pnt[0]) + sqr(rO[1]-pnt[1]) + sqr(rO[2] -pnt[2])- sqr(r);

  double det = sqr(b) - 4 * a * c;

  if (det < 0)
    return -1;

  det = sqrt(det);

  double t0 = (-b - det) / (2*a);
  if (t0 >= 0)
    return t0;

  double t1 = (-b + det) / (2*a);
  if (t1 >= 0)
    return t1;

  return -1;
}

Pixel* raycast(Object** objects, int pxW, int pxH)
{
  double cx = 0; //camera location
  double cy = 0;
  double ch = 0; //initialize camera frame size
  double cw = 0;

  int i = 0;
  while (objects[i] != NULL) {
    if(objects[i]->kind == 2)
    {
      cw = objects[i]->camera.width;
      ch = objects[i]->camera.height;
      i++;
      break;
    }
  }
  if(cw == 0 || ch == 0)
  {
    fprintf(stderr, "Error: No camera defined ");
    exit(1);
  }
  double pixHeight = ch / pxH; //size of the pixels
  double pixWidth = cw / pxW;

  double rO[3] = {cx, cy, 0}; //origin of ray

  Pixel* image;
  image = malloc(sizeof(Pixel) * pxW * pxH); //Prepare memory for image data
  for (int y = pxH; y >= 0; y -= 1) {
    for (int x = 0; x < pxW; x += 1) {
      double rD[3] = {cx - (cw/2) + pixWidth * (x + 0.5),cy - (ch/2) + pixHeight * (y + 0.5),1.0}; //location of current pixel

      normalize(rD);

      double bestT = INFINITY; //initialize the best intersection
      int bestO = -1;
      double* color;
      int i = 0;
      while(objects[i] != NULL) // check all objects for intersection
      {
	       double t = 0;
	        switch(objects[i]->kind)
          {
	           case 0:
	            t = planeIntersect(objects[i],rO, rD);
	           break;
             case 1:
              t = sphereIntersect(objects[i],rO, rD);
             break;
             case 2:
             break;
             default:
             // Horrible error
             fprintf(stderr, "Error: invalid object %i\n", objects[i]->kind);
              exit(1);
	        }
          if (t > 0 && t < bestT) // if the object is closer, replace as new best
          {
            bestT = t;
            bestO = i;
          }
          i++;
        }
        if (bestT > 0 && bestT != INFINITY) // Collect color data
        {
          switch(objects[bestO]->kind) // check object type
          {
             case 0:
              color = objects[bestO]->plane.color;
             break;
             case 1:
              color = objects[bestO]->sphere.color;
             break;
             case 2:
             break;
             default:
             // Horrible error
             fprintf(stderr, "Error: invalid object\n");
              exit(1);
          }
          image[pxH*(pxH - y-1) + x].r = color[0]*255; // store color data
          image[pxH*(pxH - y-1) + x].g = color[1]*255;
          image[pxH*(pxH - y-1) + x].b = color[2]*255;
        }
        else
        {

        }
      }
  }

  return image;
}
//Parsing JSON
Object** parseScene(char* input)
{

  int c;
  int objectI = 0;
  Object** objects;
  objects = malloc(sizeof(Object*)*129); //create object array

  FILE* json = fopen(input,"r"); // read file
  if (json == NULL)
  {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", input);
    exit(1);
  }

  checkNextChar(json, '['); //first char should be [

  while(1)
  {
    c = getC(json);
    if (c == ']') // if end of file - done
    {
      fprintf(stderr, "Error: Empty scene file.\n");
      fclose(json);
      exit(1);
    }
    if (c == '{') // if new object
    {
      checkNextString(json,"type");
      checkNextChar(json,':');
      char* value = nextString(json);
      objects[objectI] = malloc(sizeof(Object));
      if (strcmp(value, "camera") == 0) //Get type of object
      {
        objects[objectI]->kind = 2;
      }
      else if (strcmp(value, "sphere") == 0)
      {
        objects[objectI]->kind = 1;
      }
      else if (strcmp(value, "plane") == 0)
      {
        objects[objectI]->kind = 0;
      }
      else
      {
        fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
        fclose(json);
        exit(1);
      }

      while (1)
      {
        c = nextChar(json);
        if (c == '}') // if end of object
        {
      	  // stop parsing this object
          objectI++; // move to next object index
      	  break;
      	}
        else if (c == ',') // if there is another parameter
        {
      	  // read another field
      	  char* key = nextString(json);
      	  checkNextChar(json, ':');
      	  if ((strcmp(key, "width") == 0)) //save values
              {
      	    double value = nextNumber(json);
            if (objects[objectI]->kind == 2) {
              objects[objectI]->camera.width = value;
            }
            else
            {
              fprintf(stderr, "Error: Invalid property, \"%s\", on line number %d.\n", key, line);
              fclose(json);
              exit(1);
            }
      	      }
          else if ((strcmp(key, "height") == 0))
              {
      	    double value = nextNumber(json);
              if (objects[objectI]->kind == 2) {
                objects[objectI]->camera.height = value;
              }
              else
              {
                fprintf(stderr, "Error: Invalid property, \"%s\", on line number %d.\n", key, line);
                fclose(json);
                exit(1);
              }
      	      }
          else if ((strcmp(key, "radius") == 0))
              {
          	    double value = nextNumber(json);
                if (objects[objectI]->kind == 1)
                {
                  objects[objectI]->sphere.radius = value;
                }
                else
                {
                  fprintf(stderr, "Error: Invalid property, \"%s\", on line number %d.\n", key, line);
                  fclose(json);
                  exit(1);
                }
      	      }
          //Vectors
          else if ((strcmp(key, "color") == 0))
               {
      	          double* value = nextVector(json);
                  if (objects[objectI]->kind == 1)
                  {
                    for(int i = 0;i<3;i++)
                    {
                    objects[objectI]->sphere.color[i] = value[i];
                    }
                  }
                  else if (objects[objectI]->kind == 0)
                  {
                    for(int i = 0;i<3;i++)
                    {
                    objects[objectI]->plane.color[i] = value[i];
                    }
                  }
                  else
                  {
                    fprintf(stderr, "Error: Invalid property, \"%s\", on line number %d.\n", key, line);
                    fclose(json);
                    exit(1);
                  }
      	       }
          else if ((strcmp(key, "position") == 0))
              {
                double* value = nextVector(json);
                if (objects[objectI]->kind == 1)
                {
                  for(int i = 0;i<3;i++)
                  {
                  objects[objectI]->sphere.position[i] = value[i];
                  }
                }
                else if (objects[objectI]->kind == 0)
                {
                  for(int i = 0;i<3;i++)
                  {
                  objects[objectI]->plane.position[i] = value[i];
                  }
                }
                else
                {
                  fprintf(stderr, "Error: Invalid property, \"%s\", on line number %d.\n", key, line);
                  fclose(json);
                  exit(1);
                }
              }
         else if ((strcmp(key, "normal") == 0))
              {
                double* value = nextVector(json);
                if (objects[objectI]->kind == 0)
                {
                  for(int i = 0;i<3;i++)
                  {
                  objects[objectI]->plane.normal[i] = value[i];
                  }
                }
                else
                {
                  fprintf(stderr, "Error: Invalid property, \"%s\", on line number %d.\n", key, line);
                  fclose(json);
                  exit(1);
                }
              }
          else {
      	    fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n", key, line);
            fclose(json);
              exit(1);
      	    //char* value = next_string(json);
      	       }

      	}
        else
        {
      	  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
          fclose(json);
          exit(1);
      	}
      }
      c = nextChar(json);
      if (c == ',')
      {
        // noop
      }
      else if (c == ']') //if end of file
      {
        objects[objectI] = NULL;
        fclose(json);
        return objects;
      }
      else
      {
        fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
        fclose(json);
        exit(1);
      }
    }
  }
}

int getC(FILE* json)
{
  int c = fgetc(json); // get the character

  if (c == '\n') // count the number of lines
  {
    line += 1;
  }

  if (c == EOF)
  {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}
// Grabs the next non whitespace character from the file and returns it.
int nextChar(FILE* json)
{
  int c = getC(json);
  while (isspace(c))
  {
    c = getC(json);
  }
  return c;
}
//check the character that is next, throw error if not correct
int checkNextChar(FILE* json, int val)
{
  int c = nextChar(json);
  if(c==val)
  {
    return c;
  }
  else
  {
    fprintf(stderr, "Error: Expected '%c' on line %d.\n", val, line);
    exit(1);
  }
}
//collect the characters until the next "
char* nextString(FILE* json)
{
  char buffer[129];
  int c = checkNextChar(json,'"');
  c = nextChar(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = nextChar(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}
//check if the string is the one defined.
char* checkNextString(FILE* json, char* value)
{
  char* key = nextString(json);
  if (strcmp(key, value) != 0)
  {
    fprintf(stderr, "Error: Expected %s key on line number %d.\n", value, line);
    exit(1);
  }
  else
  {
    return key;
  }
}
//parse the values in a vector
double* nextVector(FILE* json)
{
  double* v = malloc(3*sizeof(double));
  checkNextChar(json, '[');
  v[0] = nextNumber(json);
  checkNextChar(json, ',');
  v[1] = nextNumber(json);
  checkNextChar(json, ',');
  v[2] = nextNumber(json);
  checkNextChar(json, ']');
  return v;
}
//collect next numeric value
double nextNumber(FILE* json)
{
  float value;
  fscanf(json, "%f", &value);
  return value;
}
