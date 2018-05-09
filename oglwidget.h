#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QWidget>
#include <QtOpenGL>
#include <QOpenGLWidget>

#include <GL/glut.h>

#include <vector>
#include<entities/model.h>

#include <bib/CameraDistante.h>
#include <bib/Desenha.h>

using namespace  std;

class OGLWidget : public QGLWidget
{
public:
    OGLWidget(QWidget *parent = 0);
    void initializeGL();
    void paintGL();
        void displayInit();
        void displayEnd();
    void resizeGL(int w, int h);

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void addTorusListaModelos();
    void addTeapotListaModelos();

    void addKratosListaModelos();
    void addBoyListaModelos();
    void addMarioListaModelos();
    void addShelfListaModelos();

    void increaseCont();
    void decreaseCont();

    void mudancasEixoX(char tipo, float valor);
    void mudancasEixoY(char tipo, float valor);
    void mudancasEixoZ(char tipo, float valor);

    void mudancasAngulo(float x, float y, float z);
    void mudancasTranslacao(float x, float y, float z);
    void mudancasEscala(float x, float y, float z);

    void carregarEstado();
    void carregarModelo3DOBJ(string caminho, string nome);
    void salvarEstado();


    ~OGLWidget();

public slots:
    void buttonPressEvent();

private:
    vector<Model*> listaModelos;

    int cont = 0;

    int width = 800;
    int height = 600;

    int slices = 20;
    int stacks = 20;

    float trans_obj = false;

    /*

    float tx = 0.0;
    float ty = 0.0;
    float tz = 0.0;

    float ax = 0.0;
    float ay = 0.0;
    float az = 0.0;

    */
    float delta = 5.0;

    float sx = 1.0;
    float sy = 1.0;
    float sz = 1.0;



    Camera* cam = new CameraDistante();
    float savedCamera[9];

    QTimer timer;
    QPoint lastPos;

protected:

};

#endif // OGLWIDGET_H
