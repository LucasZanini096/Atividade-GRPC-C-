#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "notas.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;

class NotasClient {
public:
    NotasClient(std::shared_ptr<Channel> channel)
        : stub_(gerencia_notas::GerenciadorNotas::NewStub(channel)) {}

    bool AdicionarNota(const std::string& ra, const std::string& cod_disciplina, int ano, int semestre, float nota) {
        gerencia_notas::AdicionaNotaRequest request;
        request.set_ra(ra);
        request.set_cod_disciplina(cod_disciplina);
        request.set_ano(ano);
        request.set_semestre(semestre);
        request.set_nota(nota);

        gerencia_notas::StatusResponse response;
        ClientContext context;

        Status status = stub_->AdicionarNota(&context, request, &response);

        if (status.ok()) {
            std::cout << "Resultado: " << (response.sucesso() ? "Sucesso" : "Falha") 
                      << " - " << response.msg() << std::endl;
            return response.sucesso();
        } else {
            std::cout << "RPC falhou: " << status.error_code() << ": " << status.error_message() << std::endl;
            return false;
        }
    }

    bool AlterarNota(const std::string& ra, const std::string& cod_disciplina, int ano, int semestre, float nota) {
        gerencia_notas::AdicionaNotaRequest request;
        request.set_ra(ra);
        request.set_cod_disciplina(cod_disciplina);
        request.set_ano(ano);
        request.set_semestre(semestre);
        request.set_nota(nota);

        gerencia_notas::StatusResponse response;
        ClientContext context;

        Status status = stub_->AlterarNota(&context, request, &response);

        if (status.ok()) {
            std::cout << "Resultado: " << (response.sucesso() ? "Sucesso" : "Falha") 
                      << " - " << response.msg() << std::endl;
            return response.sucesso();
        } else {
            std::cout << "RPC falhou: " << status.error_code() << ": " << status.error_message() << std::endl;
            return false;
        }
    }

    void ConsultarNota(const std::string& ra, const std::string& cod_disciplina) {
        gerencia_notas::AlunoDisciplinaRequest request;
        request.set_ra(ra);
        request.set_cod_disciplina(cod_disciplina);

        gerencia_notas::ConsultaNotaResponse response;
        ClientContext context;

        Status status = stub_->ConsultarNota(&context, request, &response);

        if (status.ok()) {
            if (response.sucesso()) {
                const auto& nota = response.nota();
                std::cout << "Nota encontrada: RA=" << nota.ra() 
                          << ", Disciplina=" << nota.cod_disciplina()
                          << ", Ano=" << nota.ano()
                          << ", Semestre=" << nota.semestre()
                          << ", Nota=" << nota.nota() << std::endl;
            } else {
                std::cout << "Erro: " << response.msg_erro() << std::endl;
            }
        } else {
            std::cout << "RPC falhou: " << status.error_code() << ": " << status.error_message() << std::endl;
        }
    }

    void CalcularMedia(const std::string& ra) {
        gerencia_notas::AlunoRequest request;
        request.set_ra(ra);

        gerencia_notas::MediaResponse response;
        ClientContext context;

        Status status = stub_->CalcularMedia(&context, request, &response);

        if (status.ok()) {
            if (response.sucesso()) {
                std::cout << "Média do aluno " << ra << ": " << response.media() << std::endl;
            } else {
                std::cout << "Erro: " << response.msg_erro() << std::endl;
            }
        } else {
            std::cout << "RPC falhou: " << status.error_code() << ": " << status.error_message() << std::endl;
        }
    }

    void ListarNotasAluno(const std::string& ra) {
        gerencia_notas::AlunoRequest request;
        request.set_ra(ra);

        ClientContext context;
        std::unique_ptr<ClientReader<gerencia_notas::Nota>> reader(
            stub_->ListarNotasAluno(&context, request));

        gerencia_notas::Nota nota;
        std::cout << "Notas do aluno " << ra << ":" << std::endl;
        
        while (reader->Read(&nota)) {
            std::cout << "  - Disciplina: " << nota.cod_disciplina()
                      << ", Ano: " << nota.ano()
                      << ", Semestre: " << nota.semestre()
                      << ", Nota: " << nota.nota() << std::endl;
        }

        Status status = reader->Finish();
        if (!status.ok()) {
            std::cout << "RPC falhou: " << status.error_code() << ": " << status.error_message() << std::endl;
        }
    }

    void testarTudo() {
        std::cout << "\n=== TESTANDO SISTEMA DE GERENCIAMENTO DE NOTAS ===" << std::endl;
        
        std::cout << "\n1. Adicionando notas..." << std::endl;
        AdicionarNota("123", "MAT001", 2024, 1, 8.5);
        AdicionarNota("123", "FIS001", 2024, 1, 7.0);
        AdicionarNota("456", "MAT001", 2024, 1, 9.0);
        AdicionarNota("789", "QUI001", 2024, 1, 6.5);
        AdicionarNota("789", "BIO001", 2024, 1, 8.0);

        std::cout << "\n2. Consultando uma nota específica..." << std::endl;
        ConsultarNota("123", "MAT001");

        std::cout << "\n3. Alterando uma nota..." << std::endl;
        AlterarNota("123", "MAT001", 2024, 1, 9.5);

        std::cout << "\n4. Calculando médias..." << std::endl;
        CalcularMedia("123");
        CalcularMedia("789");

        std::cout << "\n5. DESAFIO: Listando todas as notas do RA 789 via streaming..." << std::endl;
        ListarNotasAluno("789");
    }

private:
    std::unique_ptr<gerencia_notas::GerenciadorNotas::Stub> stub_;
};

int main(int argc, char** argv) {
    std::string target_str = "localhost:50052";
    
    NotasClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    
    client.testarTudo();

    return 0;
}