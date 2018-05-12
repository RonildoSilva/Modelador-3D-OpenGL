#include "tdsmodelloader.h"

TdsModelLoader::TdsModelLoader(const char *name, std::string filename) {
    this->filename = filename;

    file=lib3ds_file_load(name);

    if (!file) {
      puts("3dsplayer: Error: Loading 3DS file failed.\n");
      exit(1);
    }
    lib3ds_file_eval(file,0);
}

TdsModelLoader::TdsModelLoader(std::__cxx11::string filename, std::string nomeModelo,
        float tx, float ty, float tz, float ax, float ay, float az, float sx, float sy, float sz){
    this->tx = tx;
    this->ty = ty;
    this->tz = tz;

    this->ax = ax;
    this->ay = ay;
    this->az = az;

    this->sx = sx;
    this->sy = sy;
    this->sz = sz;

    this->filename = filename;
    this->nome = nomeModelo;

    file=lib3ds_file_load(filename.c_str());

    if (!file) {
      puts("3dsplayer: Error: Loading 3DS file failed.\n");
      exit(1);
    }
    lib3ds_file_eval(file,0);
}

TdsModelLoader::~TdsModelLoader()
{
    lib3ds_file_free(file);
}

#ifdef  USE_SDL
void *convert_to_RGB_Surface(SDL_Surface *bitmap)
{
  unsigned char *pixel = (unsigned char *)malloc(sizeof(char) * 4 * bitmap->h * bitmap->w);
  int soff = 0;
  int doff = 0;
  int x, y;
  unsigned char *spixels = (unsigned char *)bitmap->pixels;
  SDL_Palette *pal = bitmap->format->palette;

  for (y = 0; y < bitmap->h; y++)
    for (x = 0; x < bitmap->w; x++)
    {
      SDL_Color* col = &pal->colors[spixels[soff]];

      pixel[doff] = col->r;
      pixel[doff+1] = col->g;
      pixel[doff+2] = col->b;
      pixel[doff+3] = 255;
      doff += 4;
      soff++;
    }

    return (void *)pixel;
}
#endif

void TdsModelLoader::render_node(Lib3dsNode *node)
{
    ASSERT(file);

    {
        Lib3dsNode *p;
        for (p=node->childs; p!=0; p=p->next) {
            render_node(p);
        }
    }
    if (node->type==LIB3DS_OBJECT_NODE) {
        Lib3dsMesh *mesh;

        if (strcmp(node->name,"$$$DUMMY")==0) {
            return;
        }

        mesh = lib3ds_file_mesh_by_name(file, node->data.object.morph);
        if( mesh == NULL )
            mesh = lib3ds_file_mesh_by_name(file, node->name);

        if (!mesh->user.d) {
            ASSERT(mesh);
            if (!mesh) {
                return;
            }

            mesh->user.d=glGenLists(1);
            glNewList(mesh->user.d, GL_COMPILE);

            {
                unsigned p;
                Lib3dsVector *normalL= (Lib3dsVector*) malloc(3*sizeof(Lib3dsVector)*mesh->faces);
                Lib3dsMaterial *oldmat = (Lib3dsMaterial *)-1;
                {
                    Lib3dsMatrix M;
                    lib3ds_matrix_copy(M, mesh->matrix);
                    lib3ds_matrix_inv(M);
                    glMultMatrixf(&M[0][0]);
                }
                lib3ds_mesh_calculate_normals(mesh, normalL);

                for (p=0; p<mesh->faces; ++p) {
                    Lib3dsFace *f=&mesh->faceL[p];
                    Lib3dsMaterial *mat=0;
#ifdef	USE_SDL
                    Player_texture *pt = NULL;
                    int tex_mode = 0;
#endif
                    if (f->material[0]) {
                        mat=lib3ds_file_material_by_name(file, f->material);
                    }

                    if( mat != oldmat ) {
                        if (mat) {
                            if( mat->two_sided )
                                glDisable(GL_CULL_FACE);
                            else
                                glEnable(GL_CULL_FACE);

                            glDisable(GL_CULL_FACE);

                            if (mat->texture1_map.name[0]) {		/* texture map? */
                                Lib3dsTextureMap *tex = &mat->texture1_map;
                                if (!tex->user.p) {		/* no player texture yet? */
                                    char texname[1024];
                                    pt = (Player_texture*) malloc(sizeof(*pt));
                                    tex->user.p = pt;
                                    strcpy(texname, tex->name);
                                    IMG_Init(IMG_INIT_PNG);
#ifdef	DEBUG
                                    printf("Loading texture map, name %s\n", texname);

#endif	/* DEBUG */
#ifdef	USE_SDL
#ifdef  USE_SDL_IMG_load
                                    pt->bitmap = IMG_load(texname);
#else
                                    pt->bitmap = IMG_Load(texname);
#endif /* IMG_Load */

#else /* USE_SDL */
                                    pt->bitmap = NULL;
                                    fputs("3dsplayer: Warning: No image loading support, skipping texture.\n", stderr);
#endif /* USE_SDL */
                                    if (pt->bitmap) {	/* could image be loaded ? */
                                        /* this OpenGL texupload code is incomplete format-wise!
                      * to make it complete, examine SDL_surface->format and
                      * tell us @lib3ds.sf.net about your improvements :-)
                      */
                                        int upload_format = GL_RED; /* safe choice, shows errors */
#ifdef USE_SDL
                                        int bytespp = pt->bitmap->format->BytesPerPixel;
                                        void *pixel = NULL;
                                        glGenTextures(1, &pt->tex_id);
#ifdef	DEBUG
                                        printf("Uploading texture to OpenGL, id %d, at %d bytepp\n",
                                               pt->tex_id, bytespp);
#endif	/* DEBUG */
                                        if (pt->bitmap->format->palette) {
                                            pixel = convert_to_RGB_Surface(pt->bitmap);
                                            upload_format = GL_RGBA;
                                        }
                                        else {
                                            pixel = pt->bitmap->pixels;
                                            /* e.g. this could also be a color palette */
                                            if (bytespp == 1) upload_format = GL_LUMINANCE;
                                            else if (bytespp == 3) upload_format = GL_RGB;
                                            else if (bytespp == 4) upload_format = GL_RGBA;
                                        }
                                        glBindTexture(GL_TEXTURE_2D, pt->tex_id);
                                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                                                     TEX_XSIZE, TEX_YSIZE, 0,
                                                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                                        glTexParameteri(GL_TEXTURE_2D,
                                                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                        glTexParameteri(GL_TEXTURE_2D,
                                                        GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                                        glTexSubImage2D(GL_TEXTURE_2D,
                                                        0, 0, 0, pt->bitmap->w, pt->bitmap->h,
                                                        upload_format, GL_UNSIGNED_BYTE, pixel);
                                        pt->scale_x = (float)pt->bitmap->w/(float)TEX_XSIZE;
                                        pt->scale_y = (float)pt->bitmap->h/(float)TEX_YSIZE;
#endif /* USE_SDL */
                                        pt->valid = 1;
                                    }
                                    else {
                                        fprintf(stderr,
                                                "Load of texture %s did not succeed "
                                                "(format not supported !)\n",
                                                texname);
                                        pt->valid = 0;
                                    }
                                }
                                else {
                                    pt = (Player_texture *)tex->user.p;
                                }
                                tex_mode = pt->valid;
                            }
                            else {
                                tex_mode = 0;
                            }
                            glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
                            glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
                            glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
                            glMaterialf(GL_FRONT, GL_SHININESS, pow(2, 10.0*mat->shininess));
                        }
                        else {
                            static const Lib3dsRgba a={0.5, 0.5, 0.5, 1.0};
                            static const Lib3dsRgba d={0.5, 0.5, 0.5, 1.0};
                            static const Lib3dsRgba s={1.0, 1.0, 1.0, 1.0};
                            glMaterialfv(GL_FRONT, GL_AMBIENT, a);
                            glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
                            glMaterialfv(GL_FRONT, GL_SPECULAR, s);
                            glMaterialf(GL_FRONT, GL_SHININESS, pow(2, 10.0*0.5));
                        }
                        oldmat = mat;
                    }

                    else if (mat != NULL && mat->texture1_map.name[0]) {
                        Lib3dsTextureMap *tex = &mat->texture1_map;
                        if (tex != NULL && tex->user.p != NULL) {
                            pt = (Player_texture *)tex->user.p;
                            tex_mode = pt->valid;
                        }
                    }


                    {
                        int i;

                        if (tex_mode) {
                            //printf("Binding texture %d\n", pt->tex_id);
                            glEnable(GL_TEXTURE_2D);
                            glBindTexture(GL_TEXTURE_2D, pt->tex_id);
                        }

                        glBegin(GL_TRIANGLES);
                        glNormal3fv(f->normal);
                        for (i=0; i<3; ++i) {
                            glNormal3fv(normalL[3*p+i]);

                            if (tex_mode) {
                                glTexCoord2f(mesh->texelL[f->points[i]][1]*pt->scale_x,
                                        pt->scale_y - mesh->texelL[f->points[i]][0]*pt->scale_y);
                            }

                            glVertex3fv(mesh->pointL[f->points[i]].pos);
                        }
                        glEnd();

                        if (tex_mode)
                            glDisable(GL_TEXTURE_2D);
                    }
                }

                free(normalL);
            }

            glEndList();
        }

        if (mesh->user.d) {
            Lib3dsObjectData *d;

            glPushMatrix();
            d=&node->data.object;
            glMultMatrixf(&node->matrix[0][0]);
            glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);
            glCallList(mesh->user.d);
            /* glutSolidSphere(50.0, 20,20); */
            glPopMatrix();
        }
    }
}

void TdsModelLoader::desenha()
{
    glPushMatrix();

    //composicao de transformacoes
    glTranslated(this->getTX(),this->getTY(),this->getTZ());

    glRotated(this->getAX(),0,0,1);
    glRotated(this->getAY(),0,1,0);
    glRotated(this->getAZ(),1,0,0);

    glScaled(this->getSX(),this->getSY(),this->getSZ());

    //desenhando eixos do sistema de coordenadas local 1
    if(isEixo()){
        Desenha::drawEixos( 0.5 );
    }

    //desenhando objeto

    if(isSelecionado()){
        glColor3f(0.22,1.0,0.07);
    }
    else{
        glColor3f(0.5,0.5,0.5);
    }

    for (Lib3dsNode *node = file->nodes; node!=0; node=node->next) {
        render_node(node);
    }

    glPopMatrix();
}

float TdsModelLoader::getAX() { return this->ax; }
float TdsModelLoader::getAY() { return this->ay; }
float TdsModelLoader::getAZ() { return this->az; }

float TdsModelLoader::getSize() {}
void TdsModelLoader::setSize(float size){}

void TdsModelLoader::addAX(float ax) { this->ax+=ax; }
void TdsModelLoader::addAY(float ay) { this->ay+=ay; }
void TdsModelLoader::addAZ(float az) { this->az+=az; }

void TdsModelLoader::setAX(float ax) { this->ax = ax; }
void TdsModelLoader::setAY(float ay) { this->ay = ay; }
void TdsModelLoader::setAZ(float az) { this->az = az; }

void TdsModelLoader::addSX(float sx) { this->sx+=sx; }
void TdsModelLoader::addSY(float sy) { this->sy+=sy; }
void TdsModelLoader::addSZ(float sz) { this->sz+=sz; }

void TdsModelLoader::setSX(float sx) { this->sx = sx; }
void TdsModelLoader::setSY(float sy) { this->sy = sy; }
void TdsModelLoader::setSZ(float sz) { this->sz = sz; }

float TdsModelLoader::getSX() { return this->sx; }
float TdsModelLoader::getSY() { return this->sy; }
float TdsModelLoader::getSZ() { return this->sz; }

void TdsModelLoader::setTX(float tx) { this->tx = tx; }
void TdsModelLoader::setTY(float ty) { this->ty = ty; }
void TdsModelLoader::setTZ(float tz) { this->tz = tz; }

float TdsModelLoader::getTX() { return this->tx; }
float TdsModelLoader::getTY() { return this->ty; }
float TdsModelLoader::getTZ() { return this->tz; }

void TdsModelLoader::addTX(float tx) { this->tx+=tx; }
void TdsModelLoader::addTY(float ty) { this->ty+=ty; }
void TdsModelLoader::addTZ(float tz) { this->tz+=tz; }

void TdsModelLoader::addSlices(){}
void TdsModelLoader::addStacks(){}

void TdsModelLoader::decSlices(){}
void TdsModelLoader::decStacks(){}
int TdsModelLoader::getSlices(){}
int TdsModelLoader::getStacks(){}

void TdsModelLoader::setSelecionado(bool selecionado) { this->selecionado = selecionado; }
bool TdsModelLoader::isSelecionado() { return this->selecionado; }

void TdsModelLoader::setEixo(bool eixo) { this->eixo = eixo; }
bool TdsModelLoader::isEixo() { return this->eixo; }

std::string TdsModelLoader::getNome() { return this->nome; }
