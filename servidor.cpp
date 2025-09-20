#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "notas.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

// Implementação do serviço
class GerenciadorNotasImpl final : public gerencia_notas::GerenciadorNotas::Service {
private:
    // Estrutura para armazenar notas em memória
    std::map<std::string, std::vector<gerencia_notas::Nota>> notas_por_aluno;
    
public:
    Status AdicionarNota(ServerContext* context, const gerencia_notas::AdicionaNotaRequest* request, gerencia_notas::StatusResponse* response) override {
        std::cout << "Adicionando nota para RA: " << request->ra() << std::endl;
        
        // Cria uma nova nota
        gerencia_notas::Nota nova_nota;
        nova_nota.set_ra(request->ra());
        nova_nota.set_cod_disciplina(request->cod_disciplina());
        nova_nota.set_ano(request->ano());
        nova_nota.set_semestre(request->semestre());
        nova_nota.set_nota(request->nota());
        
        // Adiciona à estrutura de dados
        notas_por_aluno[request->ra()].push_back(nova_nota);
        
        response->set_sucesso(true);
        response->set_msg("Nota adicionada com sucesso");
        
        return Status::OK;
    }

    Status AlterarNota(ServerContext* context, const gerencia_notas::AdicionaNotaRequest* request, gerencia_notas::StatusResponse* response) override {
        std::cout << "Alterando nota para RA: " << request->ra() << std::endl;
        
        // Busca a nota existente
        auto& notas = notas_por_aluno[request->ra()];
        bool encontrou = false;
        
        for (auto& nota : notas) {
            if (nota.cod_disciplina() == request->cod_disciplina() && 
                nota.ano() == request->ano() && 
                nota.semestre() == request->semestre()) {
                nota.set_nota(request->nota());
                encontrou = true;
                break;
            }
        }
        
        if (encontrou) {
            response->set_sucesso(true);
            response->set_msg("Nota alterada com sucesso");
        } else {
            response->set_sucesso(false);
            response->set_msg("Nota não encontrada");
        }
        
        return Status::OK;
    }

    Status ConsultarNota(ServerContext* context, const gerencia_notas::AlunoDisciplinaRequest* request, gerencia_notas::ConsultaNotaResponse* response) override {
        std::cout << "Consultando nota para RA: " << request->ra() << ", Disciplina: " << request->cod_disciplina() << std::endl;
        
        auto it = notas_por_aluno.find(request->ra());
        if (it != notas_por_aluno.end()) {
            for (const auto& nota : it->second) {
                if (nota.cod_disciplina() == request->cod_disciplina()) {
                    response->set_sucesso(true);
                    *response->mutable_nota() = nota;
                    return Status::OK;
                }
            }
        }
        
        response->set_sucesso(false);
        response->set_msg_erro("Nota não encontrada");
        return Status::OK;
    }

    Status CalcularMedia(ServerContext* context, const gerencia_notas::AlunoRequest* request, gerencia_notas::MediaResponse* response) override {
        std::cout << "Calculando média para RA: " << request->ra() << std::endl;
        
        auto it = notas_por_aluno.find(request->ra());
        if (it != notas_por_aluno.end() && !it->second.empty()) {
            float soma = 0.0;
            int count = 0;
            
            for (const auto& nota : it->second) {
                soma += nota.nota();
                count++;
            }
            
            float media = soma / count;
            response->set_sucesso(true);
            response->set_media(media);
        } else {
            response->set_sucesso(false);
            response->set_media(0.0);
            response->set_msg_erro("Nenhuma nota encontrada para o aluno");
        }
        
        return Status::OK;
    }

    Status ListarNotasAluno(ServerContext* context, const gerencia_notas::AlunoRequest* request, ServerWriter<gerencia_notas::Nota>* writer) override {
        std::cout << "Listando notas para RA: " << request->ra() << std::endl;
        
        auto it = notas_por_aluno.find(request->ra());
        if (it != notas_por_aluno.end()) {
            for (const auto& nota : it->second) {
                writer->Write(nota);
            }
        }
        
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50052");
    GerenciadorNotasImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Servidor rodando em " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}