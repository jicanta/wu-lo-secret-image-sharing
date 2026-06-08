#!/usr/bin/env bash
#
# probar.sh — verifica de punta a punta que visualSSS funciona.
#
# Corre:
#   1. compilacion limpia (make clean && make)
#   2. tests unitarios (make test)
#   3. ciclo distribuir -> recuperar para k = 2..10
#   4. caso catedra k=8 (solo recuperar)
#
# Uso:  ./probar.sh
#
set -u

# --- ubicarse en el directorio del script ---
cd "$(dirname "$0")" || exit 1

TMP="$(mktemp -d /tmp/visualSSS.XXXXXX)"
SECRETO="portadoras/secreto.bmp"
fallos=0

ok()   { printf '  \033[32m[OK]\033[0m   %s\n' "$1"; }
fail() { printf '  \033[31m[FALLO]\033[0m %s\n' "$1"; fallos=$((fallos+1)); }
titulo() { printf '\n\033[1m=== %s ===\033[0m\n' "$1"; }

cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

# --- 1. compilacion limpia ---
titulo "1. Compilacion limpia"
make clean >/dev/null 2>&1
if make 2>"$TMP/build.log"; then
    if [ -s "$TMP/build.log" ]; then
        fail "compilo pero con warnings:"
        cat "$TMP/build.log"
    else
        ok "compila sin warnings"
    fi
else
    fail "no compila"
    cat "$TMP/build.log"
    echo "Abortando: sin binario no se puede seguir."
    exit 1
fi

# --- 2. tests unitarios ---
titulo "2. Tests unitarios"
if make test 2>"$TMP/test.log"; then
    grep "all tests passed" "$TMP/test.log" || true
    ok "tests unitarios pasaron"
else
    fail "tests unitarios fallaron"
    cat "$TMP/test.log"
fi

# --- 3. ciclo distribuir -> recuperar para k = 2..10 ---
titulo "3. Distribuir -> recuperar (k = 2..10)"
total_secreto=$(wc -c < "$SECRETO")
for k in 2 3 4 5 6 7 8 9 10; do
    src="portadoras/k$k"
    if [ ! -d "$src" ]; then
        fail "k=$k: no existe $src"
        continue
    fi
    work="$TMP/k$k"
    cp -r "$src" "$work"
    rec="$TMP/recuperada_k$k.bmp"

    if ! ./visualSSS -d -secret "$SECRETO" -k "$k" -dir "$work" >/dev/null 2>&1; then
        fail "k=$k: distribuir fallo"
        continue
    fi
    if ! ./visualSSS -r -secret "$rec" -k "$k" -dir "$work" >/dev/null 2>&1; then
        fail "k=$k: recuperar fallo"
        continue
    fi
    if [ ! -f "$rec" ]; then
        fail "k=$k: no se genero la imagen recuperada"
        continue
    fi
    # diferencia de bytes (esperado ~0.4% por el manejo del valor 256)
    distintos=$(cmp -l "$SECRETO" "$rec" 2>/dev/null | wc -l)
    pct=$(awk -v d="$distintos" -v t="$total_secreto" 'BEGIN{printf "%.2f", (t>0)? d*100.0/t : 0}')
    # toleramos hasta 2% de bytes distintos
    if awk -v p="$pct" 'BEGIN{exit !(p<=2.0)}'; then
        ok "k=$k: round-trip correcto ($distintos bytes distintos, $pct%)"
    else
        fail "k=$k: demasiada diferencia ($distintos bytes, $pct%)"
    fi
done

# --- 4. caso catedra k=8 (solo recuperar) ---
titulo "4. Caso catedra k=8 (solo recuperar)"
rec_cat="$TMP/recuperada_catedra.bmp"
if ./visualSSS -r -secret "$rec_cat" -k 8 -dir portadoras/k8 >/dev/null 2>&1 && [ -f "$rec_cat" ]; then
    ok "recuperacion desde portadoras de la catedra OK"
else
    fail "no se pudo recuperar el secreto de la catedra"
fi

# --- resumen ---
titulo "Resumen"
if [ "$fallos" -eq 0 ]; then
    printf '\033[32mTodo OK.\033[0m\n'
    exit 0
else
    printf '\033[31m%d verificacion(es) fallaron.\033[0m\n' "$fallos"
    exit 1
fi
