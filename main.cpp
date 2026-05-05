#include <iostream>            
#include <fstream>             
#include <sstream>             
#include <vector>              // for the dynamic arrays.
#include <cmath>              
#include <limits>             
#include <string>              
#include <cstdlib>             
#include <unordered_map>       // used fore mapping material ids to material objects.

using namespace std;
const float eps = 1e-6;   // used this const to avoid floatingpoint precision issues.     


class Vec3 {   //a class for 3 dim. vectors which will have the operations as well
public:
    float x, y, z;
    Vec3(float a = 0, float b = 0, float c = 0) : x(a), y(b), z(c) {}  //default vals for a 3dim vector

    
    Vec3 operator+(const Vec3& v) const   //overload operator for the addition of the 3dim vectors
        { return Vec3(x + v.x, y + v.y, z + v.z); }

    
    Vec3 operator-(const Vec3& v) const  // Overload operator for vector subtraction.
        { return Vec3(x - v.x, y - v.y, z - v.z); }

    
    Vec3 operator*(float s) const       // overload operator for scalar multiplication.
        { return Vec3(x * s, y * s, z * s); }

    
    Vec3 operator/(float s) const      // overload operator for scalar division.
        { return Vec3(x / s, y / s, z / s); }

    
    Vec3 mult(const Vec3& v) const    // component-wise multiplication.
        { return Vec3(x * v.x, y * v.y, z * v.z); }

    
    float dot(const Vec3& v) const   // dot product of two vectors.
        { return x * v.x + y * v.y + z * v.z; }


    
    Vec3 cross(const Vec3& v) const {  //cross product of two vectors.
        return Vec3(y * v.z - z * v.y,
                    z * v.x - x * v.z,
                    x * v.y - y * v.x);
    }


    
    Vec3 normalize() const {  // normalizing the vector.
        float len = sqrt(x * x + y * y + z * z);
        if (len > eps)
            return (*this) / len;
        return Vec3(0, 0, 0);
    }
};




class Ray {      //ray = o + tD
public:
    Vec3 origin;      // origine point of the ray.
    Vec3 direction;   // ray direction - (will have to be normalized to scale "t" correctly for intersections).
    
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.normalize()) {}  //consructor normalizing the direction
};





class Material {
public:
    Vec3 ambient;   // ambient reflectance, also a 3dim vector (r, g, b)
    Vec3 diffuse;   // diffused reflectance, also a 3dim vector (r, g, b)
    Vec3 specular;  // specular/highlight reflectance, also a 3dim vector (r, g, b)
    float phongExp; // phong exponent for specular highlights.
    Vec3 mirror;    // mirror reflectance (r, g, b)
    
    Material() : ambient(), diffuse(), specular(), phongExp(0), mirror() {}  // default constructor initializes all to zero.
};




class Light {   //point light class
public:
    Vec3 position;   // position of the light.
    Vec3 intensity;  // light intensity.  will be calculated later, in the shading step
    Light(const Vec3& p, const Vec3& i) : position(p), intensity(i) {}
};




struct HitRecord {   //will use it to store the intersections data
    float t;           // distance to intersection
    Vec3 hitPoint;     // intersection point
    Vec3 normal;       // surface normal at the hit point
    Material material; // material of the object hit
    bool hit;          // flag indicating whether an intersection occurred
    HitRecord() : t(numeric_limits<float>::max()), hitPoint(), normal(), material(), hit(false) {}
};






class Object {     //abstract object class
public:
    Material material; // material for the object. "material class"
    virtual ~Object() = default;
    virtual bool intersect(const Ray& ray, HitRecord& rec) const = 0;   //for the intersection test
};



class Sphere : public Object {  //(d.d)(t^2) + 2d.(o-c)t + (o-c).(o-c) -  (R^2)  = 0  ---> from slides
public:
    Vec3 center;   // centere of the sphere
    float radius;  // radius of the sphere
    
    Sphere(const Vec3& c, float r, const Material& m) : center(c), radius(r) { material = m; }   // to set the center, radius, and material
    
    virtual bool intersect(const Ray& ray, HitRecord& rec) const {   // intersection function for a sphere
        Vec3 oc = ray.origin - center;                  // vector from ray origin to sphere center
        float a = ray.direction.dot(ray.direction);     // d.d 
        float b = 2.0f * oc.dot(ray.direction);         // 2d.(o-c)
        float c = oc.dot(oc) - radius * radius;         //(o-c).(o-c) -  (R^2)
        float discriminant = b * b - 4 * a * c;         // discriminant, to test for roots


        if (discriminant < 0)// No real roots: no intersection. 
            return false;  


        float sqrtDisc = sqrt(discriminant);               // square root of the discriminante
        float t1 = (-b - sqrtDisc) / (2 * a);              // first possible sol for intersection
        float t2 = (-b + sqrtDisc) / (2 * a);              // second possible intersection

        // choose the closer positive t
        float t = std::min(t1, t2);
        if (t < eps)  //t is too small, the intersection is too close to the rays origin.
            return false; 

        rec.t = t;                                       // hit distance.
        rec.hitPoint = ray.origin + ray.direction * t;   // compute intersection point "ray equation"
        rec.normal = (rec.hitPoint - center).normalize();  // compute surface normal.
        rec.material = material;                         // seting material.
        rec.hit = true;                                  // mark hit "the flag" as true.
        return true;
    }
};





class Triangle : public Object { 
public:
    Vec3 v0, v1, v2;  // Vertices of the triangle.
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c, const Material& m): v0(a), v1(b), v2(c) {
        material = m;}


    // I used moller-trumbore algorithm which is faster than the "Implicit Method" in our slides; Refrence for the Algorithm:https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection.html
    virtual bool intersect(const Ray& ray, HitRecord& rec) const {
        Vec3 edge1 = v1 - v0;                         
        Vec3 edge2 = v2 - v0;                        
        Vec3 h = ray.direction.cross(edge2);          // cross product of ray direction and edge2
        float a = edge1.dot(h);                       //determinante
        if (fabs(a) < eps)                            // ray is parallel to triangle.
            return false;   

        float f = 1.0f / a;
        Vec3 s = ray.origin - v0;                       // vector from v0 to ray origin
        float u = f * s.dot(h);                         // barycentric coordinate u
        if (u < 0.0f || u > 1.0f)                       // Intersection lies outside the triangle
            return false;    


        Vec3 q = s.cross(edge1);                        
        float v = f * ray.direction.dot(q);             // barycentrice coordinate v
        if (v < 0.0f || u + v > 1.0f)                  // Intersection lies outside the triangle.
            return false;  


        float t = f * edge2.dot(q);                   // intersection distance  
        if (t < eps)                                  // Intersection behind the ray origin.
            return false;          

        rec.t = t;                                      
        rec.hitPoint = ray.origin + ray.direction * t;  // intersection point.
        rec.normal = edge1.cross(edge2).normalize();    // surface normal.
        rec.material = material;                        // set material.
        rec.hit = true;                                 // hit flag as true.
        return true;
    }
};




//Global vars.  "To be read from the input file"
vector<Object*> objects;   // list of scene object
vector<Light> lights;      // list of point lights
Vec3 ambientLight;         // ambient light (r, g, b)
Vec3 backgroundColor;      // background color (r, g, b)
int maxRecursionDepth;     
float shadowRayEpsilon;    

// camera parameters storeed in a struct. "according to input file format"
struct Camera {
    Vec3 position;      // camera position.
    Vec3 gaze;          // camera gaze direction.
    Vec3 up;            // up vector.
    float left, right, bottom, top; // image plane boundaries.
    float nearDistance; // distance from camera to image plane.
    int imageWidth, imageHeight;    // resolution of the output image.
} camera;


vector<Vec3> vertices;   // vertex list for use by spheres, triangles, and meshes


unordered_map<int, Material> materialMap;  //material map to store materials by their id.



//functions
void parseInput(const string& filename);
bool trace(const Ray& ray, HitRecord& closestHit);   //to find closest intersection of a ray
Vec3 shade(const Ray& ray, int depth);               //computing colors of ray from :diffused light, ambient + highlits + mirror reflections
void renderScenee(const string& outputFilename);      //ray trace each pixel and writes to output file
Vec3 reflect(const Vec3& I, const Vec3& N);         //computes reflection direction


bool trace(const Ray& ray, HitRecord& closestHit) {
    bool hitflage = false;
    closestHit.t = numeric_limits<float>::max();
    HitRecord tempRec;
    
    for (size_t i = 0; i < objects.size(); i++) {   //for every object in scene
        if (objects[i]->intersect(ray, tempRec)) {
            if (tempRec.t < closestHit.t) { // if this intersection is closer.
                closestHit = tempRec;
                hitflage = true;
            }
        }
    }
    return hitflage;
}




Vec3 reflect(const Vec3& I, const Vec3& N) {
    return I - N * (2 * I.dot(N)); // reflection formula (slides).
}



Vec3 shade(const Ray& ray, int depth) {
    HitRecord rec;
    if (trace(ray, rec)) {  // if an object was hit.
        Vec3 color(0, 0, 0);
        //ambient for each chanel: I = Ka * Ia . (slides) 
        color.x += rec.material.ambient.x * ambientLight.x;
        color.y += rec.material.ambient.y * ambientLight.y;
        color.z += rec.material.ambient.z * ambientLight.z;

              
        // loop over all lights. which will each contribute to diffused ,shadows, and highlights
        for (size_t i = 0; i < lights.size(); i++) {
            Light light = lights[i];
            Vec3 lightVec = light.position - rec.hitPoint;
            float dist2 = lightVec.dot(lightVec); // Squared distance to light.
            Vec3 lightDir = lightVec.normalize();

            // shadow ray: offset the hit point to avoid self-intersection. Algorithm refrence: "scratchapixel" site
            Ray shadowRay(rec.hitPoint + rec.normal * shadowRayEpsilon, lightDir);
            HitRecord shadowRec;
            if (trace(shadowRay, shadowRec)) {
                if (shadowRec.t * shadowRec.t < dist2)
                    continue; // the point is in shadow for this light.
            }
                
            // diffuse shading using lambert law (slides). I = Kd * max(0,wi.n) 
            float NdotL = max(rec.normal.dot(lightDir), 0.0f);
            Vec3 diffuse(rec.material.diffuse.x * NdotL,
                         rec.material.diffuse.y * NdotL,
                         rec.material.diffuse.z * NdotL); 

           
            // specular shading using blinn-phong model(slides). I =Ks *  (max(0, N.H))^P
            Vec3 viewDir = (ray.origin - rec.hitPoint).normalize();
            Vec3 halfDir = (lightDir + viewDir).normalize();
            float NdotH = max(rec.normal.dot(halfDir), 0.0f);
            float specFactor = pow(NdotH, rec.material.phongExp);
            Vec3 specular(rec.material.specular.x * specFactor,
                          rec.material.specular.y * specFactor,
                          rec.material.specular.z * specFactor);        

           
            float attenuation = 1.0f / dist2;   // attenuation
            color.x += (diffuse.x + specular.x) * attenuation * light.intensity.x;
            color.y += (diffuse.y + specular.y) * attenuation * light.intensity.y;
            color.z += (diffuse.z + specular.z) * attenuation * light.intensity.z;                  
        }      
           
            
        
        if (depth < maxRecursionDepth && (rec.material.mirror.x > 0 || rec.material.mirror.y > 0 || rec.material.mirror.z > 0)) {   // mirror reflection depends on material
            Vec3 reflectedDir = reflect(ray.direction, rec.normal).normalize();
            // Offset the origin to avoid self-intersection.
            Ray reflectedRay(rec.hitPoint + rec.normal * shadowRayEpsilon, reflectedDir);
            Vec3 reflectedColor = shade(reflectedRay, depth + 1);
            color.x += reflectedColor.x * rec.material.mirror.x;
            color.y += reflectedColor.y * rec.material.mirror.y;
            color.z += reflectedColor.z * rec.material.mirror.z;  
        }         
        
        return color;
    }
   
    
    return backgroundColor;     // if no hit, just return the background color
}



void renderScenee(const string& outputFilename) {
    vector<Vec3> image(camera.imageWidth * camera.imageHeight);

    // compute camera vectors (slides). "cross product for each pair will give the 3rd" 
    Vec3 w = camera.gaze.normalize(); // here I took it in the direction of the camera is looking, so basically its (-w) according to formula we took
    Vec3 u = w.cross(camera.up).normalize();
    Vec3 v = u.cross(w).normalize();

    
    Vec3 center = camera.position + w * camera.nearDistance; // m = e + (-w)*distance

    // for every pixel on the plane/ image
    for (int j = 0; j < camera.imageHeight; j++) {
        for (int i = 0; i < camera.imageWidth; i++) {
            // su = (r-I)(i+.5)/nx
            // sv = (t-b)(j+.5)/ny
            float uscale = (float(i) + 0.5f) / camera.imageWidth;   
            float vscale = (float(j) + 0.5f) / camera.imageHeight;  
            float su_off = camera.left + (camera.right - camera.left) * uscale;
            float sv_off = camera.bottom + (camera.top - camera.bottom) * vscale;
            Vec3 q = center + u * su_off + v * sv_off;  //q = m + lu + tv
            
            Vec3 rayDir = (q - camera.position).normalize(); 
            Ray ray(camera.position, rayDir);  // ray from camera to pixel
            Vec3 pixelColor = shade(ray, 0);   // shading stage (diffuse, ambient , highlights, reflections..)

            // limiting color values to [0,255].
            pixelColor.x = min(255.0f, max(0.0f, pixelColor.x));
            pixelColor.y = min(255.0f, max(0.0f, pixelColor.y));
            pixelColor.z = min(255.0f, max(0.0f, pixelColor.z));
          
            image[(camera.imageHeight - 1 - j) * camera.imageWidth + i] = pixelColor;  //image showed originaly at bottom left so I had to flip it
        }
    }

    
    ofstream ofs(outputFilename); //write the img to the output file
    if (!ofs) {
        cerr << "fil cant be opened" << endl;
        exit(1);
    }
    ofs << "P3\n" << camera.imageWidth << " " << camera.imageHeight << "\n255\n";
    for (size_t i = 0; i < image.size(); i++) {
        ofs << int(image[i].x) << " " << int(image[i].y) << " " << int(image[i].z) << "\n";
    }
    ofs.close();
    cout << "completed the renderinge, saved to: " << outputFilename << endl;
}


void parseInput(const string& filename) {
    ifstream ifs(filename);
    if (!ifs) {
        cerr << "canet open the input file: " << filename << endl;
        exit(1);
    }
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        //according input file format
        if (line[0] == '#') {
            if (line == "#BackgroundColor") {
                getline(ifs, line);
                istringstream(line) >> backgroundColor.x >> backgroundColor.y >> backgroundColor.z;
            }
            else if (line == "#MaxRecursionDepth") {
                getline(ifs, line);
                istringstream(line) >> maxRecursionDepth;
            }
            else if (line == "#ShadowRayEpsilon") {
                getline(ifs, line);
                istringstream(line) >> shadowRayEpsilon;
            }
            else if (line == "#Camera") {
                
                getline(ifs, line);
                istringstream(line) >> camera.position.x >> camera.position.y >> camera.position.z;
                getline(ifs, line);
                istringstream(line) >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
                getline(ifs, line);
                istringstream(line) >> camera.up.x >> camera.up.y >> camera.up.z;
                getline(ifs, line);
                istringstream(line) >> camera.left >> camera.right >> camera.bottom >> camera.top;
                getline(ifs, line);
                istringstream(line) >> camera.nearDistance;
                getline(ifs, line);
                istringstream(line) >> camera.imageWidth >> camera.imageHeight;
            }
            else if (line == "#Material") {
                int materialId;
                getline(ifs, line);
                istringstream(line) >> materialId; // read material id.
                Material mat;
                getline(ifs, line);
                istringstream(line) >> mat.ambient.x >> mat.ambient.y >> mat.ambient.z;
                getline(ifs, line);
                istringstream(line) >> mat.diffuse.x >> mat.diffuse.y >> mat.diffuse.z;
                getline(ifs, line);
                istringstream(line) >> mat.specular.x >> mat.specular.y >> mat.specular.z;
                getline(ifs, line);
                istringstream(line) >> mat.phongExp;
                getline(ifs, line);
                istringstream(line) >> mat.mirror.x >> mat.mirror.y >> mat.mirror.z;
                
                materialMap[materialId] = mat;// store material in the map using its id.
            }
            else if (line == "#AmbientLight") {
                getline(ifs, line);
                istringstream(line) >> ambientLight.x >> ambientLight.y >> ambientLight.z;
            }
            else if (line == "#PointLight") {
                int lightId;
                getline(ifs, line);
                istringstream(line) >> lightId; 
                Vec3 pos, intens;
                getline(ifs, line);
                istringstream(line) >> pos.x >> pos.y >> pos.z;
                getline(ifs, line);
                istringstream(line) >> intens.x >> intens.y >> intens.z;
                lights.push_back(Light(pos, intens));
            }
            else if (line == "#VertexList") {
                
                while (ifs.peek() != EOF) {  // will read vertices until the next tag is encountered
                    char c = ifs.peek();
                    if (c == '#' || c == '\n')
                        break;
                    getline(ifs, line);
                    if (line.empty()) continue;
                    Vec3 vertex;
                    istringstream iss(line);
                    if (iss >> vertex.x >> vertex.y >> vertex.z)
                        vertices.push_back(vertex);
                }
            }
            else if (line == "#Sphere") {
                int sphereId;
                getline(ifs, line);
                istringstream(line) >> sphereId;
                int materialId;
                getline(ifs, line);
                istringstream(line) >> materialId;
                int vertexIndex;
                getline(ifs, line);
                istringstream(line) >> vertexIndex;
                float radius;
                getline(ifs, line);
                istringstream(line) >> radius;
  
                Material mat = materialMap[materialId];  // using the material correspond to materialId.
                Vec3 center = vertices[vertexIndex - 1]; // from 1 index to 0 index.. more natural to deal with
                objects.push_back(new Sphere(center, radius, mat));
            }
            else if (line == "#Triangle") {
                int triangleId;
                getline(ifs, line);
                istringstream(line) >> triangleId;
                int materialId;
                getline(ifs, line);
                istringstream(line) >> materialId;
                int i1, i2, i3;
                getline(ifs, line);
                istringstream(line) >> i1 >> i2 >> i3;
                Material mat = materialMap[materialId];
                Vec3 a = vertices[i1 - 1];
                Vec3 b = vertices[i2 - 1];
                Vec3 c = vertices[i3 - 1];
                objects.push_back(new Triangle(a, b, c, mat));
            }
            else if (line == "#Mesh") {
                int meshId;
                getline(ifs, line);
                istringstream(line) >> meshId;
                int materialId;
                getline(ifs, line);
                istringstream(line) >> materialId;
                Material mat = materialMap[materialId];
                // Read mesh faces until a new tag is encountered.
                while (ifs.peek() != EOF) {
                    if (ifs.peek() == '#' || ifs.peek() == '\n')
                        break;
                    getline(ifs, line);
                    if (line.empty()) continue;
                    int i1, i2, i3;
                    istringstream iss(line);
                    if (!(iss >> i1 >> i2 >> i3))
                        break;
                    Vec3 a = vertices[i1 - 1];
                    Vec3 b = vertices[i2 - 1];
                    Vec3 c = vertices[i3 - 1];
                    objects.push_back(new Triangle(a, b, c, mat));
                }
            }
        }
    }
    ifs.close();
}


int main(int argc, char* argv[]) {
    
    if (argc < 2) {// Checks if input file is provided.
        cerr << "usee ./Raytracer input.txt" << endl;
        return 1;
    }
    
    parseInput(argv[1]);// parses the scene from the input file "given in command line"
    renderScenee("output.ppm");//renders the scene and write the output image.

    
    for (size_t i = 0; i < objects.size(); i++) {  // clean up dynamically allocated objects.
        delete objects[i];
    }
    return 0;
}
