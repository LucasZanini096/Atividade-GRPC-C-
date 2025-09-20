# servidor.py (adaptado)
import grpc
from concurrent import futures
import notas_pb2
import notas_pb2_grpc

# Armazenamento em memória (simulando um banco de dados)
db_notas = {} # Ex: { "123_DISC1": Nota(...), "123_DISC2": Nota(...) }

class GerenciadorNotasServicer(notas_pb2_grpc.GerenciadorNotasServicer):

    def AdicionarNota(self, request, context):
        print(f"Adicionando nota para RA {request.ra} na disciplina {request.cod_disciplina}")
        chave = f"{request.ra}_{request.cod_disciplina}"

        if chave in db_notas:
            return notas_pb2.StatusResponse(sucesso=False, msg="Nota já existe para este aluno/disciplina. Use AlterarNota.")

        # Criando a Nota
        nota = notas_pb2.Nota(
        ra=request.ra,
        cod_disciplina=request.cod_disciplina,
        ano=request.ano,
        semestre=request.semestre,
        nota=request.nota
        )
        db_notas[chave] = nota
        return notas_pb2.StatusResponse(sucesso=True, msg="Nota adicionada com sucesso!")

    def AlterarNota(self, request, context):
        print(f"Alterando nota para RA {request.ra} na disciplina {request.cod_disciplina}")
        chave = f"{request.ra}_{request.cod_disciplina}"

        if chave not in db_notas:
            return notas_pb2.StatusResponse(sucesso=False, msg="Nota não encontrada. Adicione a nota antes de alterar.")

# Alterando a nota
        nota = db_notas[chave]
        nota.nota = request.nota # Atualizando o valor da nota
        nota.ano = request.ano
        nota.semestre = request.semestre
        db_notas[chave] = nota
        return notas_pb2.StatusResponse(sucesso=True, msg="Nota alterada com sucesso!")

    def ConsultarNota(self, request, context):
        print(f"Consultando nota para RA {request.ra} na disciplina {request.cod_disciplina}")
        chave = f"{request.ra}_{request.cod_disciplina}"

        if chave not in db_notas:
            return notas_pb2.ConsultaNotaResponse(sucesso=False, msg_erro="Nota não encontrada.")

        # Retornando a nota consultada
        nota = db_notas[chave]
        return notas_pb2.ConsultaNotaResponse(sucesso=True, nota=nota, msg_erro="")

    def CalcularMedia(self, request, context):
        print(f"Calculando a média para RA {request.ra}")
        soma_notas = 0
        contador = 0

        # Itera pelas notas do aluno
        for chave, nota in db_notas.items():
            if nota.ra == request.ra:
                soma_notas += nota.nota
                contador += 1

        if contador == 0:
            return notas_pb2.MediaResponse(sucesso=False, media=0, msg_erro="Nenhuma nota encontrada para o RA informado.")

        media = soma_notas / contador
        return notas_pb2.MediaResponse(sucesso=True, media=media, msg_erro="")

    def ListarNotasAluno(self, request, context):
        """
        Este método utiliza streaming do lado do servidor.
        Ele envia as notas de um aluno uma por uma.
        """
        
        print(f"Listando notas para o RA {request.ra}")

        # Itera sobre o banco de dados e retorna as notas do aluno
        for chave, nota in db_notas.items():
            if nota.ra == request.ra:
                yield nota

            # Se o aluno não tiver notas, envia um erro
            if not any(nota.ra == request.ra for chave, nota in db_notas.items()):
                context.set_code(grpc.StatusCode.NOT_FOUND)
                context.set_details(f"Nenhuma nota encontrada para o RA {request.ra}")
                yield notas_pb2.Nota() # Retorna uma nota vazia como resposta


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    notas_pb2_grpc.add_GerenciadorNotasServicer_to_server(GerenciadorNotasServicer(), server)
    server.add_insecure_port('[::]:50052')
    server.start()
    print("Servidor gRPC rodando na porta 50052.")
    server.wait_for_termination()

if __name__ == '__main__':
    serve()