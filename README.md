# visualSSS — Secreto compartido en imágenes con esteganografía

Implementación en C del esquema de secreto compartido de Wu y Lo. Reparte una imagen secreta BMP en n imágenes portadoras (sombras): con cualquier k de ellas se reconstruye el secreto y con menos de k no se obtiene nada. Los datos de cada sombra quedan escondidos dentro de imágenes de aspecto normal mediante esteganografía LSB.

## Compilación

Requiere gcc y C99.

```
make            # genera el binario visualSSS
make test       # corre los tests unitarios
make clean
```

## Uso

```
./visualSSS -d -secret <imagen.bmp> -k <num> [-n <num>] [-dir <dir>]
./visualSSS -r -secret <imagen.bmp> -k <num> [-n <num>] [-dir <dir>]
```

| Parámetro | Significado |
|---|---|
| `-d` | Distribuir, esconde el secreto en las portadoras |
| `-r` | Recuperar, reconstruye el secreto a partir de las sombras |
| `-secret` | BMP a esconder con `-d`, o archivo de salida con `-r` |
| `-k` | Sombras mínimas para reconstruir, entre 2 y 10 |
| `-n` | Total de sombras a generar, por defecto la cantidad de BMP que haya en `-dir` |
| `-dir` | Directorio de portadoras, por defecto el directorio actual |

Los parámetros obligatorios (`-d` o `-r`, `-secret` y `-k`) deben ir en ese orden. Ante un error en los parámetros o en las imágenes, el programa informa el motivo y no hace nada.

## Ejemplos de prueba

El directorio `portadoras/` trae todo lo necesario para probar de k=2 hasta k=10. Adentro está `secreto.bmp`, que es la imagen recuperada de la cátedra (la Torre Eiffel), y una carpeta por cada k (`k2`, `k3`, hasta `k10`) con las portadoras del tamaño que ese k necesita. Las portadoras son retratos como Einstein o Marilyn, para que se note que las sombras parecen fotos comunes.

Distribuir sobrescribe las portadoras, así que conviene trabajar sobre una copia. Por ejemplo, para k=5:

```
cp -r portadoras/k5 /tmp/k5
./visualSSS -d -secret portadoras/secreto.bmp -k 5 -dir /tmp/k5     # encripta
./visualSSS -r -secret /tmp/recuperada.bmp -k 5 -dir /tmp/k5        # desencripta
```

La imagen `/tmp/recuperada.bmp` se ve igual que `portadoras/secreto.bmp`. Para los demás casos se cambia el 5 por el k deseado, y `-n` toma por defecto la cantidad de portadoras de la carpeta. La recuperada difiere en cerca del 0,4% de los píxeles por el manejo del valor 256, pero a la vista es la misma imagen.

Para k=8 las portadoras son directamente los archivos de la cátedra, que ya vienen con el secreto distribuido, así que ese caso solo se desencripta.

```
./visualSSS -r -secret /tmp/recuperada.bmp -k 8 -dir portadoras/k8
```

## Requisitos de las imágenes

- BMP de 8 bits por píxel, en escala de grises, sin compresión y sin datos extra después de los píxeles.
- Todas las portadoras de un mismo reparto deben tener el mismo tamaño.
- Cada portadora necesita al menos 8·⌈m/k⌉ píxeles, donde m es la cantidad de píxeles del secreto. Para k=8 eso equivale al tamaño del secreto.
