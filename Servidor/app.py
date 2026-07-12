import psycopg2
from flask import Flask, render_template, request, redirect, jsonify, flash

app = Flask(__name__)

app.secret_key = "chave_secreta_pop-up_smartbar_1012"

def conectar_banco():
    return psycopg2.connect(
        host="postgresql.janks.dev.br",       
        database="projeto_b",     
        user="iot",    
        password="pepcon-garton"   
    )

@app.route("/")
def funcao_da_pagina_inicio():
    return render_template("inicio.html")

@app.route("/form_cliente")
def form_cliente():
    return render_template("cadastro_cliente.html")

@app.route("/form_copo")
def form_copo():
    return render_template("cadastro_copo.html")

@app.route('/cadastrar_copo', methods=['POST'])
def tratar_copo():
    id_fisico_copo = request.form.get('id_copo')
    rfid_copo = request.form.get('rfid_copo').upper()

    caracteres_hex = "0123456789abcdefABCDEF"
    rfid_valido = True

    if len(rfid_copo) != 11:
        rfid_valido = False

    elif rfid_copo[2] != ' ' or rfid_copo[5] != ' ' or rfid_copo[8] != ' ':
        rfid_valido = False

    else:
        pos_carac = [0,1,3,4,6,7,9,10]

        for pos in pos_carac:
            if rfid_copo[pos] not in caracteres_hex:
                rfid_valido = False
                break

    if (rfid_valido == False):
        flash(f"Erro: Tentativa de cadastro com RFID inválido: {rfid_copo}")
        return redirect('/erro_copo')
    
    try:
        conn = conectar_banco()
        cur = conn.cursor()
        
        sql = "INSERT INTO public.copos (id_copo, status, rfid, nivel_bateria) VALUES (%s, 'disponivel', %s, '100');"
        cur.execute(sql, (id_fisico_copo, rfid_copo,))
        
        conn.commit() 
        cur.close()
        conn.close()
        
        return redirect('/sucesso_copo')
    except Exception as e:
        print(f"Erro ao cadastrar copo: {e}")
        return redirect('/erro_copo')


@app.route('/cadastrar',methods=['POST'])
def tratar_cliente():
    nome = request.form.get('nome')
    telefone = request.form.get('telefone')
    email = request.form.get('email')
    
    try:
        conn = conectar_banco()
        cur = conn.cursor()
                     
        sql_cliente = "INSERT INTO public.clientes (nome_completo, telefone, email) VALUES (%s, %s, %s);"        
        cur.execute(sql_cliente, (nome, telefone, email,))
        
        conn.commit()
        cur.close()
        conn.close()
        
        return redirect('/sucesso_cliente')
    except Exception as e:
        print(f"Erro ao salvar: {e}")
        return redirect('/erro_cliente')

@app.route("/sucesso_cliente")
def mostrar_pagina_sucesso_cliente():
    return render_template("sucesso_cliente.html")

@app.route("/erro_cliente")
def mostrar_pagina_erro_cliente():
    return render_template("erro_cliente.html")

@app.route("/sucesso_copo")
def mostrar_pagina_sucesso_copo():
    return render_template("sucesso_copo.html")

@app.route("/erro_copo")
def mostrar_pagina_erro_copo():
    return render_template("erro_copo.html")


@app.route("/listagem")
def mostra_lista():

    try:
        conn = conectar_banco()
        cur = conn.cursor()

        sql = """
            SELECT c.id_copo, c.status, v.id_cliente, cl.nome_completo
            FROM public.copos c
            LEFT JOIN public.vinculo_copo_cliente v ON c.id = v.id_copo
            LEFT JOIN public.clientes cl ON v.id_cliente = cl.id
            ORDER BY c.id_copo ASC;
        """

        cur.execute(sql)

        lista_copos = cur.fetchall()

        cur.close()
        conn.close()

        print(lista_copos)
        return render_template("listagem.html", copos=lista_copos)
    
    except Exception as e:
        print(f"Erro ao buscar na lista: {e}")
        return "Erro ao carrregar a lista"

@app.route('/clientes', methods=['GET'])
def buscar_clientes():

    termo_busca = request.args.get('q', '')

    try:

        conn = conectar_banco()
        cur = conn.cursor()

        if termo_busca:

            sql = """
                SELECT 
                    c.id, c.nome_completo, c.email,
                    COALESCE(STRING_AGG(cp.id_copo::text, ', '), 'Nenhum') AS copos_ativos
                FROM public.clientes c
                LEFT JOIN public.vinculo_copo_cliente v ON c.id = v.id_cliente
                LEFT JOIN public.copos cp ON cp.id = v.id_copo AND cp.status = 'em_uso'
                WHERE c.nome_completo ILIKE %s OR c.email ILIKE %s
                GROUP BY c.id, c.nome_completo, c.email
                ORDER BY c.nome_completo;
            """
            cur.execute(sql,(f'%{termo_busca}%',f'%{termo_busca}%'))
            lista_clientes = cur.fetchall()

        else:
            
            lista_clientes = []
        
        
        cur.close()
        conn.close()

        return render_template('lista_clientes.html', clientes = lista_clientes, busca_atual = termo_busca)

    except Exception as e:
        print(f"Erro ao buscar clientes: {e}")
        return "Erro ao carregar a paagina de clientes"

@app.route("/vincular_copos", methods=['GET'])
def pagina_vincular():
    id_cliente = request.args.get('id_cliente')
    nome_cleinte = "Cliente não encontrado"
    
    try:
        if id_cliente:
            conn = conectar_banco()
            cur = conn.cursor()
            
            sql = "SELECT nome_completo FROM public.clientes WHERE id = %s;"
            cur.execute(sql, (id_cliente,))
            resultado = cur.fetchone()
            
            if resultado:
                nome_cliente = resultado[0] 
                
            cur.close()
            conn.close()
            
    except Exception as e:
        print(f"Erro ao buscar nome do cliente: {e}")

    return render_template("vincular_copos.html", id_cliente=id_cliente, nome_cliente=nome_cliente)

@app.route("/processar_vinculo", methods = ['POST'])
def processar_vinculo():
    id_cliente = request.form.get('id_cliente')
    id_fisico_copo = request.form.get('id_copo')
    
    try:
        conn = conectar_banco()
        cur = conn.cursor()
        
        sql_verifica = "SELECT id, status FROM public.copos WHERE id_copo = %s;"
        cur.execute(sql_verifica, (id_fisico_copo,))
        resultado = cur.fetchone()
        
        if resultado is None:
            cur.close()
            conn.close()
            flash(f"Erro: O copo {id_fisico_copo} não existe no sistema!")
            return redirect(f'/vincular_copos?id_cliente={id_cliente}')

        id_interno_banco_copo = resultado[0]  
        status_atual = resultado[1]
        
        if status_atual == 'em_uso':
            cur.close()
            conn.close()
            flash(f"Atenção: O copo {id_copo} já está em uso por outro cliente!")
            
            return redirect(f'/vincular_copos?id_cliente={id_cliente}')
        
        sql_update_copo = "UPDATE public.copos SET status = 'em_uso' WHERE id = %s;"
        cur.execute(sql_update_copo,(id_interno_banco_copo,))

        sql_vinculo = "INSERT INTO public.vinculo_copo_cliente (id_cliente, id_copo) VALUES (%s, %s);"
        cur.execute(sql_vinculo, (id_cliente, id_interno_banco_copo,))
        
        sql_vinculo_c = "UPDATE public.copos SET id_cliente = %s WHERE id = %s;"
        cur.execute(sql_vinculo_c, (id_cliente, id_interno_banco_copo,))

        conn.commit()
        cur.close()
        conn.close()    
        
        flash(f"Sucesso! Copo {id_fisico_copo} associado corretamente.")
        return redirect(f'/vincular_copos?id_cliente={id_cliente}')
        
    except Exception as e:
        print(f"Erro ao vincular: {e}")
        return "Erro no vinculo"


app.run(port=5000, debug=True)