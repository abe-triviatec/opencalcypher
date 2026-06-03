#include <openfhe.h>

#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/bfvrns/bfvrns-ser.h"
#include "scheme/bgvrns/bgvrns-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"

#include <openssl/sha.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace lbcrypto;

namespace {

struct Args {
    std::string command;
    std::vector<std::string> values;
};

struct ContextSpec {
    std::string scheme = "BFV";
    int security = 128;
    int depth = 2;
    int batchSize = 4096;
    uint64_t plaintextModulus = 65537;
    int scalingBits = 50;
};

const std::string b64chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void debug(const std::string& message) {
    if (std::getenv("OPENCALCYPHER_DEBUG")) std::cerr << "[opencalcypher] " << message << "\n";
}

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot read file: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void writeFile(const std::string& path, const std::string& data) {
    std::filesystem::path target(path);
    if (target.has_parent_path()) std::filesystem::create_directories(target.parent_path());
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("cannot write file: " + path);
    out << data;
}

std::string getArg(const Args& args, const std::string& name, const std::string& fallback = "") {
    for (size_t i = 0; i + 1 < args.values.size(); ++i) {
        if (args.values[i] == name) return args.values[i + 1];
    }
    return fallback;
}

bool hasArg(const Args& args, const std::string& name) {
    return std::find(args.values.begin(), args.values.end(), name) != args.values.end();
}

std::string upper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

std::string jsonString(const std::string& json, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return "";
    size_t end = pos + 1;
    while (end < json.size()) {
        if (json[end] == '"' && json[end - 1] != '\\') return json.substr(pos + 1, end - pos - 1);
        ++end;
    }
    return "";
}

int jsonInt(const std::string& json, const std::string& key, int fallback) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (!std::regex_search(json, match, pattern)) return fallback;
    return std::stoi(match[1].str());
}

uint64_t jsonUint64(const std::string& json, const std::string& key, uint64_t fallback) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (!std::regex_search(json, match, pattern)) return fallback;
    return static_cast<uint64_t>(std::stoull(match[1].str()));
}

bool jsonBool(const std::string& json, const std::string& key, bool fallback) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (!std::regex_search(json, match, pattern)) return fallback;
    return match[1].str() == "true";
}

std::string base64Encode(const std::string& input) {
    std::string out;
    int val = 0;
    int valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string base64Decode(const std::string& input) {
    std::vector<int> table(256, -1);
    for (int i = 0; i < 64; ++i) table[static_cast<unsigned char>(b64chars[i])] = i;
    std::string out;
    int val = 0;
    int valb = -8;
    for (unsigned char c : input) {
        if (c == '=') break;
        if (table[c] == -1) continue;
        val = (val << 6) + table[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);
    std::ostringstream out;
    out << std::hex;
    for (unsigned char b : hash) {
        out.width(2);
        out.fill('0');
        out << static_cast<int>(b);
    }
    return out.str();
}

template <typename T>
std::string serializeToBinaryString(const T& value) {
    std::stringstream ss;
    Serial::Serialize(value, ss, SerType::BINARY);
    return ss.str();
}

template <typename T>
T deserializeFromBinaryString(const std::string& data) {
    std::stringstream ss(data);
    T value;
    Serial::Deserialize(value, ss, SerType::BINARY);
    return value;
}

ContextSpec loadContextSpec(const std::string& path) {
    const std::string json = readFile(path);
    ContextSpec spec;
    spec.scheme = upper(jsonString(json, "scheme"));
    if (spec.scheme.empty()) spec.scheme = "BFV";
    spec.security = jsonInt(json, "security", 128);
    spec.depth = jsonInt(json, "depth", 2);
    spec.batchSize = jsonInt(json, "batchSize", 4096);
    spec.plaintextModulus = jsonUint64(json, "plaintextModulus", spec.scheme == "BGV" ? 786433 : 65537);
    spec.scalingBits = jsonInt(json, "scalingBits", 50);
    return spec;
}

CryptoContext<DCRTPoly> makeContext(const ContextSpec& spec) {
    CryptoContext<DCRTPoly> cc;
    if (spec.scheme == "BFV") {
        CCParams<CryptoContextBFVRNS> p;
        p.SetPlaintextModulus(spec.plaintextModulus);
        p.SetMultiplicativeDepth(spec.depth);
        p.SetBatchSize(spec.batchSize);
        cc = GenCryptoContext(p);
    } else if (spec.scheme == "BGV") {
        CCParams<CryptoContextBGVRNS> p;
        p.SetPlaintextModulus(spec.plaintextModulus);
        p.SetMultiplicativeDepth(spec.depth);
        p.SetBatchSize(spec.batchSize);
        cc = GenCryptoContext(p);
    } else if (spec.scheme == "CKKS") {
        CCParams<CryptoContextCKKSRNS> p;
        p.SetMultiplicativeDepth(spec.depth);
        p.SetScalingModSize(spec.scalingBits);
        p.SetBatchSize(spec.batchSize);
        p.SetScalingTechnique(FLEXIBLEAUTO);
        p.SetSecretKeyDist(UNIFORM_TERNARY);
        p.SetKeySwitchTechnique(HYBRID);
        p.SetNumLargeDigits(3);
        p.SetFirstModSize(60);
        cc = GenCryptoContext(p);
    } else {
        throw std::runtime_error("unsupported scheme: " + spec.scheme);
    }
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    return cc;
}

CryptoContext<DCRTPoly> loadContext(const std::string& path) {
    const std::string json = readFile(path);
    const std::string encodedContext = jsonString(json, "cryptoContext");
    if (encodedContext.empty()) {
        return makeContext(loadContextSpec(path));
    }
    return deserializeFromBinaryString<CryptoContext<DCRTPoly>>(base64Decode(encodedContext));
}

std::string contextJson(const ContextSpec& spec, const std::string& contextData) {
    std::ostringstream out;
    out << "{\n"
        << "  \"format\": \"opencalcypher-context-v1\",\n"
        << "  \"scheme\": \"" << spec.scheme << "\",\n"
        << "  \"security\": " << spec.security << ",\n"
        << "  \"depth\": " << spec.depth << ",\n"
        << "  \"batchSize\": " << spec.batchSize << ",\n"
        << "  \"plaintextModulus\": " << spec.plaintextModulus << ",\n"
        << "  \"scalingBits\": " << spec.scalingBits << ",\n"
        << "  \"cryptoContextSha256\": \"sha256:" << sha256(contextData) << "\",\n"
        << "  \"cryptoContextBytes\": " << contextData.size() << ",\n"
        << "  \"cryptoContext\": \"" << base64Encode(contextData) << "\"\n"
        << "}\n";
    return out.str();
}

std::string artifactPayload(const std::string& json, const std::string& field) {
    std::string encoded = jsonString(json, field);
    if (encoded.empty()) throw std::runtime_error("missing artifact field: " + field);
    return base64Decode(encoded);
}

std::string keyJson(const std::string& format, const std::string& scheme, const std::string& contextHash,
                    const std::string& keyData, bool secret) {
    std::string keyHash = sha256(keyData);
    std::ostringstream out;
    out << "{\n"
        << "  \"format\": \"" << format << "\",\n"
        << "  \"scheme\": \"" << scheme << "\",\n"
        << "  \"contextHash\": \"sha256:" << contextHash << "\",\n"
        << "  \"keyHash\": \"sha256:" << keyHash << "\",\n"
        << "  \"secretKey\": " << (secret ? "true" : "false") << ",\n"
        << "  \"keyBytes\": " << keyData.size() << ",\n"
        << "  \"key\": \"" << base64Encode(keyData) << "\"\n"
        << "}\n";
    return out.str();
}

std::string ciphertextJson(const std::string& scheme, const std::string& contextHash,
                           const std::string& keyHash, const std::string& ciphertextData) {
    std::ostringstream out;
    out << "{\n"
        << "  \"format\": \"opencalcypher-ciphertext-v1\",\n"
        << "  \"scheme\": \"" << scheme << "\",\n"
        << "  \"contextHash\": \"sha256:" << contextHash << "\",\n"
        << "  \"keyHash\": \"" << keyHash << "\",\n"
        << "  \"encoding\": \"integer\",\n"
        << "  \"ciphertextSha256\": \"" << sha256(ciphertextData) << "\",\n"
        << "  \"ciphertextBytes\": " << ciphertextData.size() << ",\n"
        << "  \"ciphertext\": \"" << base64Encode(ciphertextData) << "\",\n"
        << "  \"plaintextIncluded\": false\n"
        << "}\n";
    return out.str();
}

void usage() {
    std::cout
        << "Usage:\n"
        << "  opencalcypher context --scheme bfv --security 128 --depth 2 --batch-size 4096 --out context.json\n"
        << "  opencalcypher keygen --context context.json --out keys/\n"
        << "  opencalcypher encrypt-int --context context.json --public-key keys/public.key --value 42 --out value.ct.json\n"
        << "  opencalcypher decrypt-int --context context.json --secret-key keys/secret.key --in value.ct.json\n"
        << "  opencalcypher inspect value.ct.json\n"
        << "  opencalcypher verify --context context.json --public-key keys/public.key --in value.ct.json\n";
}

void cmdContext(const Args& args) {
    ContextSpec spec;
    spec.scheme = upper(getArg(args, "--scheme", "bfv"));
    spec.security = std::stoi(getArg(args, "--security", "128"));
    spec.depth = std::stoi(getArg(args, "--depth", "2"));
    spec.batchSize = std::stoi(getArg(args, "--batch-size", "4096"));
    spec.plaintextModulus = static_cast<uint64_t>(std::stoull(
        getArg(args, "--plaintext-modulus", spec.scheme == "BGV" ? "786433" : "65537")));
    spec.scalingBits = std::stoi(getArg(args, "--scaling-bits", "50"));
    std::string out = getArg(args, "--out");
    if (out.empty()) throw std::runtime_error("context requires --out");
    auto cc = makeContext(spec);
    writeFile(out, contextJson(spec, serializeToBinaryString(cc)));
}

void cmdKeygen(const Args& args) {
    std::string contextPath = getArg(args, "--context");
    std::string outDir = getArg(args, "--out");
    if (contextPath.empty() || outDir.empty()) throw std::runtime_error("keygen requires --context and --out");
    std::string context = readFile(contextPath);
    ContextSpec spec = loadContextSpec(contextPath);
    auto cc = loadContext(contextPath);
    auto kp = cc->KeyGen();
    std::string pub = serializeToBinaryString(kp.publicKey);
    std::string sec = serializeToBinaryString(kp.secretKey);
    std::filesystem::create_directories(outDir);
    writeFile(outDir + "/public.key", keyJson("opencalcypher-public-key-v1", spec.scheme, sha256(context), pub, false));
    writeFile(outDir + "/secret.key", keyJson("opencalcypher-secret-key-v1", spec.scheme, sha256(context), sec, true));
}

void cmdEncryptInt(const Args& args) {
    debug("encrypt-int:start");
    std::string contextPath = getArg(args, "--context");
    std::string publicKeyPath = getArg(args, "--public-key");
    std::string valueRaw = getArg(args, "--value");
    std::string out = getArg(args, "--out");
    if (contextPath.empty() || publicKeyPath.empty() || valueRaw.empty() || out.empty()) {
        throw std::runtime_error("encrypt-int requires --context --public-key --value --out");
    }
    ContextSpec spec = loadContextSpec(contextPath);
    debug("encrypt-int:context-spec-loaded");
    if (spec.scheme == "CKKS") throw std::runtime_error("encrypt-int supports exact integer schemes BFV and BGV; CKKS float support is separate");
    std::string context = readFile(contextPath);
    std::string keyJsonText = readFile(publicKeyPath);
    debug("encrypt-int:artifacts-read");
    std::string keyPayload = artifactPayload(keyJsonText, "key");
    debug("encrypt-int:key-payload-bytes=" + std::to_string(keyPayload.size()));
    auto publicKey = deserializeFromBinaryString<PublicKey<DCRTPoly>>(keyPayload);
    debug("encrypt-int:key-deserialized");
    if (!publicKey || !publicKey->GetCryptoContext()) throw std::runtime_error("public key artifact is invalid");
    auto cc = publicKey->GetCryptoContext();
    debug("encrypt-int:key-context-loaded");
    int64_t value = std::stoll(valueRaw);
    Plaintext plaintext = cc->MakePackedPlaintext({value});
    debug("encrypt-int:plaintext-built");
    auto ciphertext = cc->Encrypt(publicKey, plaintext);
    debug("encrypt-int:ciphertext-built");
    std::string ct = serializeToBinaryString(ciphertext);
    debug("encrypt-int:ciphertext-serialized");
    writeFile(out, ciphertextJson(spec.scheme, sha256(context), jsonString(keyJsonText, "keyHash"), ct));
}

void cmdDecryptInt(const Args& args) {
    std::string contextPath = getArg(args, "--context");
    std::string secretKeyPath = getArg(args, "--secret-key");
    std::string in = getArg(args, "--in");
    if (contextPath.empty() || secretKeyPath.empty() || in.empty()) {
        throw std::runtime_error("decrypt-int requires --context --secret-key --in");
    }
    ContextSpec spec = loadContextSpec(contextPath);
    if (spec.scheme == "CKKS") throw std::runtime_error("decrypt-int supports exact integer schemes BFV and BGV; CKKS float support is separate");
    auto secretKey = deserializeFromBinaryString<PrivateKey<DCRTPoly>>(artifactPayload(readFile(secretKeyPath), "key"));
    if (!secretKey || !secretKey->GetCryptoContext()) throw std::runtime_error("secret key artifact is invalid");
    auto cc = secretKey->GetCryptoContext();
    auto ciphertext = deserializeFromBinaryString<Ciphertext<DCRTPoly>>(artifactPayload(readFile(in), "ciphertext"));
    if (!ciphertext) throw std::runtime_error("ciphertext artifact is invalid");
    Plaintext plaintext;
    cc->Decrypt(secretKey, ciphertext, &plaintext);
    plaintext->SetLength(1);
    std::cout << plaintext->GetPackedValue()[0] << "\n";
}

void cmdInspect(const Args& args) {
    if (args.values.empty()) throw std::runtime_error("inspect requires an artifact path");
    std::string json = readFile(args.values[0]);
    std::cout << "{\n"
              << "  \"format\": \"" << jsonString(json, "format") << "\",\n"
              << "  \"scheme\": \"" << jsonString(json, "scheme") << "\",\n"
              << "  \"contextHash\": \"" << jsonString(json, "contextHash") << "\",\n"
              << "  \"keyHash\": \"" << jsonString(json, "keyHash") << "\",\n"
              << "  \"ciphertextSha256\": \"" << jsonString(json, "ciphertextSha256") << "\",\n"
              << "  \"ciphertextBytes\": " << jsonInt(json, "ciphertextBytes", 0) << ",\n"
              << "  \"plaintextIncluded\": " << (jsonBool(json, "plaintextIncluded", true) ? "true" : "false") << "\n"
              << "}\n";
}

void cmdVerify(const Args& args) {
    std::string contextPath = getArg(args, "--context");
    std::string publicKeyPath = getArg(args, "--public-key");
    std::string in = getArg(args, "--in");
    if (contextPath.empty() || publicKeyPath.empty() || in.empty()) {
        throw std::runtime_error("verify requires --context --public-key --in");
    }
    std::string context = readFile(contextPath);
    std::string key = readFile(publicKeyPath);
    std::string ct = readFile(in);
    std::string expectedContext = "sha256:" + sha256(context);
    if (jsonString(key, "contextHash") != expectedContext || jsonString(ct, "contextHash") != expectedContext) {
        throw std::runtime_error("artifact contextHash does not match context");
    }
    if (jsonString(ct, "keyHash") != jsonString(key, "keyHash")) {
        throw std::runtime_error("ciphertext keyHash does not match public key");
    }
    std::string raw = artifactPayload(ct, "ciphertext");
    if (jsonString(ct, "ciphertextSha256") != sha256(raw)) {
        throw std::runtime_error("ciphertextSha256 mismatch");
    }
    if (jsonBool(ct, "plaintextIncluded", true)) {
        throw std::runtime_error("plaintextIncluded must be false");
    }
    std::cout << "ok\n";
}

Args parse(int argc, char** argv) {
    Args args;
    if (argc > 1) args.command = argv[1];
    for (int i = 2; i < argc; ++i) args.values.emplace_back(argv[i]);
    return args;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        Args args = parse(argc, argv);
        if (args.command.empty() || hasArg(args, "--help") || args.command == "--help") {
            usage();
            return 0;
        }
        if (args.command == "context") cmdContext(args);
        else if (args.command == "keygen") cmdKeygen(args);
        else if (args.command == "encrypt-int") cmdEncryptInt(args);
        else if (args.command == "decrypt-int") cmdDecryptInt(args);
        else if (args.command == "inspect") cmdInspect(args);
        else if (args.command == "verify") cmdVerify(args);
        else throw std::runtime_error("unknown command: " + args.command);
        return 0;
    } catch (const std::exception& err) {
        std::cerr << "opencalcypher: " << err.what() << "\n";
        return 1;
    }
}
