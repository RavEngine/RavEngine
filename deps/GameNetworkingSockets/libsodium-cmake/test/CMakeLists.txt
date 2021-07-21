function(make_test name)
    add_executable(${name}
        ${ARGN}
    )

    target_link_libraries(${name}
        PRIVATE
            sodium
    )

    target_include_directories(${name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/libsodium/test/quirks/
    )

    add_test(
        NAME ${name}
        COMMAND ${name}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libsodium/test/default
    )
endfunction()

make_test(test_aead_aes256gcm
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/aead_aes256gcm.c
)

make_test(test_aead_aes256gcm2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/aead_aes256gcm2.c
)

make_test(test_aead_chacha20poly1305
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/aead_chacha20poly1305.c
)

make_test(test_aead_chacha20poly13052
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/aead_chacha20poly13052.c
)

make_test(test_aead_xchacha20poly1305
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/aead_xchacha20poly1305.c
)

make_test(test_auth
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/auth.c
)

make_test(test_auth2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/auth2.c
)

make_test(test_auth3
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/auth3.c
)

make_test(test_auth5
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/auth5.c
)

make_test(test_auth6
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/auth6.c
)

make_test(test_auth7
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/auth7.c
)

make_test(test_box
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box.c
)

make_test(test_box2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box2.c
)

make_test(test_box7
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box7.c
)

make_test(test_box8
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box8.c
)

make_test(test_box_easy
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box_easy.c
)

make_test(test_box_easy2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box_easy2.c
)

make_test(test_box_seal
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box_seal.c
)

make_test(test_box_seed
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/box_seed.c
)

make_test(test_chacha20
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/chacha20.c
)

make_test(test_codecs
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/codecs.c
)

make_test(test_core1
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/core1.c
)

make_test(test_core2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/core2.c
)

make_test(test_core3
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/core3.c
)

make_test(test_core4
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/core4.c
)

make_test(test_core5
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/core5.c
)

make_test(test_core6
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/core6.c
)

make_test(test_ed25519_convert
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/ed25519_convert.c
)

make_test(test_generichash
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/generichash.c
)

make_test(test_generichash2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/generichash2.c
)

make_test(test_generichash3
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/generichash3.c
)

make_test(test_hash
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/hash.c
)

make_test(test_hash3
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/hash3.c
)

make_test(test_kdf
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/kdf.c
)

make_test(test_keygen
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/keygen.c
)

make_test(test_kx
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/kx.c
)

make_test(test_metamorphic
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/metamorphic.c
)

make_test(test_misuse
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/misuse.c
)

make_test(test_onetimeauth
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/onetimeauth.c
)

make_test(test_onetimeauth2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/onetimeauth2.c
)

make_test(test_onetimeauth7
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/onetimeauth7.c
)

make_test(test_pwhash_argon2i
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/pwhash_argon2i.c
)

make_test(test_pwhash_argon2id
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/pwhash_argon2id.c
)

make_test(test_randombytes
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/randombytes.c
)

make_test(test_scalarmult
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult.c
)

make_test(test_scalarmult2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult2.c
)

make_test(test_scalarmult5
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult5.c
)

make_test(test_scalarmult6
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult6.c
)

make_test(test_scalarmult7
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult7.c
)

make_test(test_scalarmult8
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult8.c
)

make_test(test_secretbox
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretbox.c
)

make_test(test_secretbox2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretbox2.c
)

make_test(test_secretbox7
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretbox7.c
)

make_test(test_secretbox8
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretbox8.c
)

make_test(test_secretbox_easy
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretbox_easy.c
)

make_test(test_secretbox_easy2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretbox_easy2.c
)

make_test(test_secretstream
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/secretstream.c
)

make_test(test_shorthash
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/shorthash.c
)

make_test(test_sign
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/sign.c
)

make_test(test_sodium_core
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/sodium_core.c
)

make_test(test_sodium_utils
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/sodium_utils.c
)

make_test(test_sodium_utils2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/sodium_utils2.c
)

make_test(test_sodium_utils3
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/sodium_utils3.c
)

make_test(test_sodium_version
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/sodium_version.c
)

make_test(test_stream
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/stream.c
)

make_test(test_stream2
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/stream2.c
)

make_test(test_stream3
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/stream3.c
)

make_test(test_stream4
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/stream4.c
)

make_test(test_verify1
    ${PROJECT_SOURCE_DIR}/libsodium/test/default/verify1.c
)

if(NOT SODIUM_MINIMAL)
    make_test(test_core_ed25519
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/core_ed25519.c
    )

    make_test(test_core_ristretto255
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/core_ristretto255.c
    )

    make_test(test_pwhash_scrypt
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/pwhash_scrypt.c
    )

    make_test(test_pwhash_scrypt_ll
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/pwhash_scrypt_ll.c
    )

    make_test(test_scalarmult_ed25519
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult_ed25519.c
    )

    make_test(test_scalarmult_ristretto255
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/scalarmult_ristretto255.c
    )

    make_test(test_siphashx24
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/siphashx24.c
    )

    make_test(test_xchacha20
        ${PROJECT_SOURCE_DIR}/libsodium/test/default/xchacha20.c
    )
endif()
